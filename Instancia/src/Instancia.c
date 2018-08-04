#include "Instancia.h"

int main(int argc, char* argv[]) {
	int statusThread=1;
	pthread_t threadDump;

	pthread_mutex_init(&MUTEX, NULL);

	cargarConfig(argv[1]);

	conectarCoordinador();

	char * nombreArchivoLog = (char *) malloc(1 + strlen("../logs/")+ strlen(NOMBRE)+4);
	strcpy(nombreArchivoLog, "../logs/");
	strcat(nombreArchivoLog, NOMBRE);
	strcat(nombreArchivoLog,".log");


	LOG_INSTANCIA = log_create(nombreArchivoLog, NOMBRE, true, LOG_LEVEL_TRACE);
	free(nombreArchivoLog);
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");

	log_info(LOG_INSTANCIA, "Conectada a Coordinador.");


	//se crea el hilo para atender el dump
	statusThread = pthread_create(&threadDump, NULL, &atenderDump, NULL);
	if (statusThread){
		log_error(LOG_INSTANCIA, "No se pudo crear el hilo para el Dump");
	}

	PUNTERO_CIRCULAR = 0;
	CONTROL_LRU = 0;

	while(CONNECTED){ //espera que llegue una operacion
		//log_trace(LOG_INSTANCIA, "Disponible, esperando nueva operacion");
		recibirOperacion();
	}

	log_destroy(LOG_INSTANCIA);
	liberarTablaEntradas();
	liberarAlmacenamiento();
	return EXIT_SUCCESS;
}

void liberarTablaEntradas(){
	void destructor(void* dato){
		free((t_entrada*)dato);
	}
	list_destroy_and_destroy_elements(TABLA_ENTRADAS, destructor);
}

void liberarAlmacenamiento(){
	int i;
	for(i= 0;i<CANTIDAD_ENTRADAS;i++){
			free(ALMACENAMIENTO[i]);
	}
	free(ALMACENAMIENTO);
}

void test_recibirSet(char* clave, char* valor){
	memset(&paqueteSet, 0, sizeof(t_set));
	strcpy(paqueteSet.clave, clave);
	paqueteSet.valor = valor;
	paqueteSet.sizeValor = strlen(valor);
	printf("clave: %s\n", paqueteSet.clave);
	realizarSet(entradasNecesarias(paqueteSet.valor));
	//free(valor);
}

void conectarCoordinador(){
	t_head header;

	SOCKET_COORDINADOR = connectSocket(IP_COORDINADOR,PUERTO_COORDINADOR);

	// Le avisa que es una INSTANCIA
	header.context = INSTANCIA;
	header.mSize = 0;
	sendHead(SOCKET_COORDINADOR, header);

	// Le envía su nombre
	header.context = nombreInstancia;
	header.mSize = strlen(NOMBRE);
	sendHead(SOCKET_COORDINADOR, header);
	send(SOCKET_COORDINADOR, NOMBRE, strlen(NOMBRE), 0);

	CONNECTED = 1;
}

void recibirOperacion(){
	t_head header = recvHead(SOCKET_COORDINADOR);
	char unaClave[MAX_CLAVE];
	char *unValor;
	char *clavesPrevias;
	switch(header.context){
	case INIT_INSTANCIA:
		recv(SOCKET_COORDINADOR, &paqueteInit, header.mSize,0);
		CANTIDAD_ENTRADAS = paqueteInit.cantidadEntradas;
		TAMANIO_ENTRADAS = paqueteInit.sizeofEntrada;
		TABLA_ENTRADAS = list_create();
		ALMACENAMIENTO = (char**) calloc(CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
		memset(ALMACENAMIENTO, '\0', sizeof(ALMACENAMIENTO[0][0])*CANTIDAD_ENTRADAS*TAMANIO_ENTRADAS);
		log_info(LOG_INSTANCIA, "Se recibieron los datos de inicializacion");
		break;
	case OPERACION_SET:
		recvSet(SOCKET_COORDINADOR, &paqueteSet);
		pthread_mutex_lock(&MUTEX);
		realizarSet(entradasNecesarias(paqueteSet.valor));
		pthread_mutex_unlock(&MUTEX);
		log_trace(LOG_INSTANCIA,"SET-se setio la clave: %s , con el valor: %s", paqueteSet.clave, paqueteSet.valor);
		break;
	case OPERACION_STORE:
		recv(SOCKET_COORDINADOR, &paqueteStore, header.mSize, 0);
		pthread_mutex_lock(&MUTEX);
		if (realizarStore(paqueteStore.clave)){
			log_trace(LOG_INSTANCIA,"STORE- se guardo la clave %s", paqueteStore.clave);
			aumentarLRU(paqueteStore.clave);
			header.context = STORE_OK;
		} else {
			header.context = STORE_FAIL;
			log_error(LOG_INSTANCIA,"STORE - No se encontró la clave: %s", paqueteStore.clave);
		}
		pthread_mutex_unlock(&MUTEX);
		sendHead(SOCKET_COORDINADOR, header);
		break;
	case REINCORPORACION_INSTANCIA:
		//recibir el sring con todas las claves a levantar de disco
		if (header.mSize != 0){
			clavesPrevias = malloc(header.mSize);
			recv(SOCKET_COORDINADOR, clavesPrevias, header.mSize, MSG_WAITALL);
			reincorporar(clavesPrevias);
		}
		break;
	case ORDEN_COMPACTAR:
		header.context = FIN_COMPACTAR;
		log_info(LOG_INSTANCIA, "Se recibio la orden de compactar");
		compactar();
		sendHead(SOCKET_COORDINADOR, header);
		//realizarSet(entradasNecesarias(paqueteSet.valor));
		break;
	case statusValor:
		recv(SOCKET_COORDINADOR, unaClave, header.mSize, 0);
		unValor = obtenerValor(unaClave);
		header.mSize = strlen(unValor) + 1;
		sendHead(SOCKET_COORDINADOR, header);
		send(SOCKET_COORDINADOR, unValor, header.mSize, 0);
		free(unValor);
		break;
	case ERROR_HEAD:
		CONNECTED = 0;
		//procesa error
		break;
	default:
		log_info(LOG_INSTANCIA, "Se desconoce el header");
		break;
	}
}

int entradasLibre(){
	int i;
	int cantidadLibres=0;
	for(i=0;i<CANTIDAD_ENTRADAS;i++){
		if(ALMACENAMIENTO[i]==NULL){
			cantidadLibres++;
		}
	}
	return cantidadLibres;
}

void* atenderDump(){
	while(CONNECTED){
		sleep(INTERVALO_DUMP);
		pthread_mutex_lock(&MUTEX);
		realizarDump();
		pthread_mutex_unlock(&MUTEX);
		log_trace(LOG_INSTANCIA,"Se realizo el Dump");
	}
	return NULL;
}

void realizarDump(){
	void realizarDump_dato(void* dato){
		if (((t_entrada*)dato) != NULL){
			char* clave = ((t_entrada*)dato)->clave;
			realizarStore(clave);
		}
	}
	list_iterate(TABLA_ENTRADAS, realizarDump_dato);
}

int entradasNecesarias(char* valor){
	int largoString = strlen(valor);
	int cantidad = largoString/TAMANIO_ENTRADAS;
	if(cantidad*TAMANIO_ENTRADAS < largoString){
		cantidad++;
	}
	return cantidad;
}

bool hayEntradasDisponibles(int tamanio){
	int i;
	int cantidad = 0;

	for(i=0; i< CANTIDAD_ENTRADAS; i++){
		if (ALMACENAMIENTO[i] == NULL){
			cantidad++;
		}
		if (cantidad >= tamanio){
				return true;
		}
	}
	return false;
}

bool hayEspacioContiguo(int entradasNecesarias, int* posicion){
	int primeroVacio = -1;
	int i;
	for(i=0; i<CANTIDAD_ENTRADAS; i++){
		//si hay un vacio se levanta el flag primero vacio, sino se baja
		if(ALMACENAMIENTO[i]== NULL){
			if(primeroVacio == -1){
				primeroVacio = i;
			}
		}else{
			primeroVacio = -1;
		}
		//si el flag primerovacio esta arriba, se verifica si tengo las entradas necesarias
		if(primeroVacio >= 0){
			if(i-primeroVacio+1 == entradasNecesarias){
				*posicion = primeroVacio;
				return true;
			}
		}
	}
	return false;
}

void almacenarDato(int posicion, char* valor, int cantidadEntradas){
	char* valorAgrabar;
	bool seguir = true;
	int nEntrada = 0;

	while( seguir ){
		if ( strlen(valor)>TAMANIO_ENTRADAS  ){
			valorAgrabar = string_substring( valor, 0, TAMANIO_ENTRADAS );
			valor        = string_substring_from( valor, TAMANIO_ENTRADAS );
		} else {
			valorAgrabar = valor ;
			seguir = false;
		}

		ALMACENAMIENTO[posicion+nEntrada]= valorAgrabar;
		ALMACENAMIENTO[posicion+nEntrada][strlen(valorAgrabar)]= '\0';
		nEntrada++;
	}
	mostarAlmacenamiento();
}

void aumentarLRU(char* clave){
	t_entrada* dato = obtenerDato(clave);
	dato->controlLRU = CONTROL_LRU;
	CONTROL_LRU++;
}

void realizarSet_Agregar(int entradasNecesarias){
	t_entrada* dato = malloc(sizeof(t_entrada));
	int noSeAgrego = 1;
	int posicion;

	while(noSeAgrego){
		if(hayEntradasDisponibles(entradasNecesarias)){
			if(hayEspacioContiguo(entradasNecesarias, &posicion)){
				//se recibe la posicion por referencia de hayespacioscontiguos
				strcpy(dato->clave, paqueteSet.clave);
				dato->tamanio = paqueteSet.sizeValor;
				dato->cantidadUtilizada = entradasNecesarias;
				dato->posicion = posicion;
				dato->controlLRU = CONTROL_LRU;
				CONTROL_LRU++;

				list_add(TABLA_ENTRADAS, dato);
				almacenarDato(posicion, paqueteSet.valor, entradasNecesarias);
				noSeAgrego = 0;
			}else{
				log_info(LOG_INSTANCIA, "Se necesita compactar, se envia peticion al coordinador");
				enviarOrdenDeCompactar();
				recibirOperacion();
			}
		}else{
			correrAlgoritmoDeReemplazo();
		}
	}
}

void realizarSet_Actualizar(int entradasNecesarias){
	//se elimina de la lista, se pone a null las posiciones y se llama a agregar
	char* clave = paqueteSet.clave;
	t_entrada* dato;
	int i;
	int posicion;
	int cantidadEntradas;

	dato             = obtenerDato(clave);
	posicion         = dato->posicion;
	cantidadEntradas = dato->cantidadUtilizada;

	for(i=0;i<cantidadEntradas;i++){
		free(ALMACENAMIENTO[posicion+i]); // Se libera lo que estaba almacenado
		ALMACENAMIENTO[posicion+i] = NULL;
	}

	bool esElDato(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)==0){
			return true;
		}
		return false;
	}

	//free(dato); // Ya no se usa más así que se libera
	list_remove_and_destroy_by_condition(TABLA_ENTRADAS, esElDato,free);


	realizarSet_Agregar(entradasNecesarias);
}

void realizarSet(int entradasNecesarias){
	t_head headerInfo;
	headerInfo.context = NRO_ENTRADAS;
	//esta funcion agrega o modifica una entrada en la lista e entradas y
	// le envia el dato a almacenamiento
	if (yaExisteClave(paqueteSet.clave)){
		realizarSet_Actualizar(entradasNecesarias);
	}else{
		realizarSet_Agregar(entradasNecesarias);
	}
	headerInfo.mSize = entradasLibre();
	sendHead(SOCKET_COORDINADOR, headerInfo);
}

void enviarOrdenDeCompactar(){
	t_head header;

	header.context = ORDEN_COMPACTAR;
	header.mSize = 0;

	//informa al Coordinador que se necesita compactar
	sendHead(SOCKET_COORDINADOR, header);
}

bool realizarStore(char* clave){
	char* valor = obtenerValor(clave);

	if (strcmp(valor,"") != 0){
	char* archivo = string_new();
	size_t tamanio;
	string_append(&archivo, PUNTO_MONTAJE);
	string_append(&archivo, clave);
	string_append(&archivo, ".txt");
	tamanio = string_length(valor) + 1;
	//se garantiza la existencia del punto de montaje
#if defined(_WIN32)
	_mkdir(PUNTO_MONTAJE);
#else
	mkdir(PUNTO_MONTAJE, 0700);
#endif
	// Se abre el fichero a escribir.
	int fd = open( archivo, O_RDWR | O_TRUNC | O_CREAT, (mode_t) 0600);
	// Se redimensiona el fichero segun lo que se va a escribir.
	if (lseek(fd, tamanio - 1, SEEK_SET) == -1) {
		close(fd);
		log_error(LOG_INSTANCIA,"MAP - No se puedo redimensionar el fichero. Clave: %s", clave);
		free(valor);
		free(archivo);
		return false;
	}else{
	// Prueba permiso de escritura... escribiendo :)
	if (write(fd, " ", 1) == -1) {
		close(fd);
		log_error(LOG_INSTANCIA,"MAP - No se puedo escribir el fichero. Clave: %s", clave);
		free(valor);
		free(archivo);
		return false;
    }else{
   	// Ahora mapeamos (asociamos) la memoria con el espacio en disco
   	char* map = mmap(0, tamanio, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
   	if (map == MAP_FAILED) {
   		close(fd);
   		log_error(LOG_INSTANCIA,"MAP - Fallo el mmap. Clave: %s", clave);
   		free(valor);
   		free(archivo);
   		return false;
    }else{

    strcpy(map, valor);

	if (msync(map, tamanio, MS_SYNC) == -1) {
		log_error(LOG_INSTANCIA,"MAP - No se puedo sincronizar el fichero. Clave: %s", clave);
		close(fd);
		free(valor);
		free(archivo);
		return false;
	}

	if (munmap(map, tamanio) == -1) {
		close(fd);
		log_error(LOG_INSTANCIA,"MAP - Fallo el munmap. Clave: %s", clave);
		free(valor);
		free(archivo);
		return false;
	}

	close(fd);
	free(archivo);
	free(valor);
	return true;
    }}}}
	free(valor);
	return false;
}

char* obtenerValor(char* clave){
	int i;
	char* valor= string_new();
	t_entrada* dato= obtenerDato(clave);

	if (dato != NULL){
		for(i=0;i<dato->cantidadUtilizada;i++){
			string_append(&valor,ALMACENAMIENTO[dato->posicion+i]);
		}
	}else{
		log_error(LOG_INSTANCIA,"STORE - La clave %s fue reeemplazada", clave);
	}
	return valor;
}

void correrAlgoritmoDeReemplazo(){
	if (strcmp(ALGORITMO,"CIRC") == 0){
			algoritmoCircular();
	}
	if (strcmp(ALGORITMO,"LRU") == 0){
			algoritmoLRU();
	}
	if (strcmp(ALGORITMO,"BSU") == 0){
			algoritmoBSU();
	}
}

void algoritmoCircular(){
	//elimina del almacenamiento el dato atomico que encuentre, partiendo desde el valor actual
	// del puntero circular
	int mateAuno = 1;
	int cantidad;
	t_entrada* dato;
	char* clave;

	//log_info(LOG_INSTANCIA, "Se corre el algoritmo Circular");
	while(mateAuno){
		if(ALMACENAMIENTO[PUNTERO_CIRCULAR]== NULL){
			aumentarCircular();
		}else{
			dato = obtenerDato_posicion(PUNTERO_CIRCULAR);
			cantidad = dato->cantidadUtilizada;
			if (cantidad == 1){
				//es un dato atomico->lo mato
				clave = malloc(strlen(dato->clave)+1);
				//clave = strdup(dato->clave);
				strcpy(clave, dato->clave);
				ALMACENAMIENTO[PUNTERO_CIRCULAR]= NULL;

				bool esElDato(void* dato){
						if (strcmp(((t_entrada*)dato)->clave, clave)==0){
									return true;
						}
						return false;
					}
				list_remove_and_destroy_by_condition(TABLA_ENTRADAS, esElDato,free);

				aumentarCircular();
				mateAuno = 0;
				free(clave);
			}else{
				aumentarCircular();
			}
		}
	}
}

void algoritmoLRU(){
	t_entrada* datoAmatar = NULL;
	int minimo = 1000000;

	//log_info(LOG_INSTANCIA, "Se corre el algoritmo LRU");

	void minimoLRU(void* dato){
		if (((t_entrada*)dato)->cantidadUtilizada == 1){
			if (((t_entrada*)dato)->controlLRU < minimo){
				minimo = ((t_entrada*)dato)->controlLRU;
				datoAmatar= ((t_entrada*)dato);
			}
		}
	}
	list_iterate(TABLA_ENTRADAS, minimoLRU);

	if(datoAmatar == NULL){
		log_error(LOG_INSTANCIA, "No hay dato atomico para borrar");
	}else{
		char* clave = datoAmatar->clave;
		ALMACENAMIENTO[datoAmatar->posicion]= NULL;

		bool esElDato(void* dato){
				if (strcmp(((t_entrada*)dato)->clave, clave)==0){
							return true;
				}
				return false;
			}
		list_remove_and_destroy_by_condition(TABLA_ENTRADAS, esElDato, free);
	}
}

void algoritmoBSU(){
	t_entrada* datoAmatar = NULL;
	int maximo = 0;
	int controlUltimoSelecto;

	//log_info(LOG_INSTANCIA, "Se corre el algoritmo BSU");

	void maximoBSU(void* dato){
		if (((t_entrada*)dato)->cantidadUtilizada == 1){
			if (((t_entrada*)dato)->tamanio >= maximo){
				if(((t_entrada*)dato)->tamanio == maximo){
					if(((t_entrada*)dato)->controlLRU < controlUltimoSelecto){
						maximo = ((t_entrada*)dato)->tamanio;
						datoAmatar= ((t_entrada*)dato);
						controlUltimoSelecto = ((t_entrada*)dato)->controlLRU;
					}
				}else{
					maximo = ((t_entrada*)dato)->tamanio;
					datoAmatar= ((t_entrada*)dato);
					controlUltimoSelecto = ((t_entrada*)dato)->controlLRU;
				}

			}
		}
	}
	list_iterate(TABLA_ENTRADAS, maximoBSU);

	if(datoAmatar == NULL){
			log_error(LOG_INSTANCIA, "No hay dato atomico para borrar");
	}else{
		char* clave = datoAmatar->clave;
		ALMACENAMIENTO[datoAmatar->posicion]= NULL;

		bool esElDato(void* dato){
				if (strcmp(((t_entrada*)dato)->clave, clave)==0){
							return true;
				}
				return false;
			}
		list_remove_and_destroy_by_condition(TABLA_ENTRADAS, esElDato,free);
	}
}

void compactar(){
	int i;
	int j;
	int cantidadNull;
	int posicionNull;
	int buscandoNulos;
	int buscandoNulosContiguos;
	t_entrada* dato;

	bool seguir = true;

	while(seguir){
		//1- busco el primer null
		cantidadNull = 0;
		posicionNull = -1;
		buscandoNulos = 1;
		i= 0;
		while(buscandoNulos){
			if (ALMACENAMIENTO[i]==NULL){
				cantidadNull = 1;
				posicionNull = i;
				buscandoNulos= 0;
				i++;
			}else{
				i++;
				if(i>= CANTIDAD_ENTRADAS){
					buscandoNulos =0;
					seguir= false;
				}
			}
		}
		//2 - cuento nuelos contiguos
		buscandoNulosContiguos= 1;
		while(buscandoNulosContiguos){
			if (ALMACENAMIENTO[i]==NULL){
				cantidadNull++;
				i++;
			}else{
				buscandoNulosContiguos = 0;
			}
			if(i>= CANTIDAD_ENTRADAS){
				buscandoNulosContiguos =0;
				seguir= false;
			}
		}
		if(seguir){
			//3 - ubico entrada del dato siguiente a null
			dato = obtenerDato_posicion(i);
			dato->posicion -= cantidadNull;
			for(j=1;j<=dato->cantidadUtilizada;j++){
				ALMACENAMIENTO[posicionNull+j-1] = ALMACENAMIENTO[posicionNull+j-1+cantidadNull];
				ALMACENAMIENTO[posicionNull+j-1+cantidadNull] = NULL;
			}
		}
	}
}

void mostarAlmacenamiento(){
	int i;
	for(i=0;i<CANTIDAD_ENTRADAS;i++){
		log_info(LOG_INSTANCIA, "Almacenamiento [%d] = %s", i, ALMACENAMIENTO[i]);
	}
}

void reincorporar(char* claves){
	int i=0;
	char** clavesAleer = string_split(claves, ",");
	free(claves);

	while(clavesAleer[i] != NULL){
		char * valor = valorEnDisco(clavesAleer[i]);
		if (valor != NULL){
			test_recibirSet(clavesAleer[i], valor);
		}
		i++;
	}
	i=0;
	while(clavesAleer[i] != NULL){
		free(clavesAleer[i]);
		i++;
	}
	free(clavesAleer);

}

bool yaExisteClave(char* clave){
	bool claveBuscada(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)==0){
			return true;
		}
		return false;
	}
	return list_any_satisfy(TABLA_ENTRADAS, claveBuscada);
}

t_entrada* obtenerDato(char* clave){
	bool esElDato(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)==0){
					return true;
		}
		return false;
	}
	t_entrada* dato = list_find(TABLA_ENTRADAS, esElDato);
	return dato;
}

t_entrada* obtenerDato_posicion(int posicion){
	bool esElDato(void* dato){
		int posicionInicial = ((t_entrada*)dato)->posicion;
		int posicionFinal = ((t_entrada*)dato)->posicion + ((t_entrada*)dato)->cantidadUtilizada-1;
		if ((posicion >= posicionInicial) && (posicion <= posicionFinal)){
			return true;
		}
		return false;
	}
	t_entrada* dato = list_find(TABLA_ENTRADAS, esElDato);
	return dato;
}

char* valorEnDisco(char* clave){
	//en caso de que el valor retornado

	char* archivo = string_new();
	string_append(&archivo, PUNTO_MONTAJE);
	string_append(&archivo, clave);
	string_append(&archivo, ".txt");

	int fd = open(archivo, O_RDONLY, (mode_t)0600);

	if (fd == -1){
		free(archivo);
		log_error(LOG_INSTANCIA, "REINCORPORACION- La clave no fue guardada: %s", archivo);
	}else{

	struct stat fileInfo = {0};

	if (fstat(fd, &fileInfo) == -1){
		close(fd);
		free(archivo);
		log_error(LOG_INSTANCIA, "REINCORPORACION- error al obtener el tamaño del archivo");
	}else{

	if (fileInfo.st_size == 0){
		close(fd);
		free(archivo);
		log_error(LOG_INSTANCIA, "REINCORPORACION- el archivo esta vacio");
	}else{

	char *map = mmap(0, fileInfo.st_size, PROT_READ, MAP_SHARED, fd, 0);
	if (map == MAP_FAILED){
		close(fd);
		free(archivo);
		log_error(LOG_INSTANCIA, "REINCORPORACION - error al mapear");
	}else{

	int32_t tamanio = fileInfo.st_size + 1;
	char* valor = malloc(tamanio);
	memcpy(valor,map,fileInfo.st_size);
	valor[fileInfo.st_size] = '\0';

	if (munmap(map, fileInfo.st_size) == -1){
		close(fd);
		free(archivo);
		log_error(LOG_INSTANCIA, "REINCORPORACION- error al unmapear");
	}
	close(fd);
	free(archivo);
	return valor;
	}}}}
	free(archivo);
	close(fd);
	return NULL;
}
void aumentarCircular(){
	if (PUNTERO_CIRCULAR==CANTIDAD_ENTRADAS){
		PUNTERO_CIRCULAR=0;
	}
	else{
		PUNTERO_CIRCULAR++;
	}
}

