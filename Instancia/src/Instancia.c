#include "Instancia.h"

int main(int argc, char* argv[]) {
	//int i;
	int statusThread=1;
	pthread_t threadDump;

	LOG_INSTANCIA = log_create("../logs/logDeInstancia.log", "Instancia", true, LOG_LEVEL_TRACE);
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");

	cargarConfig(argv[1]);

	//se crea el hilo para atender el dump

	statusThread = pthread_create(&threadDump, NULL, &atenderDump, NULL);
	if (statusThread){
		log_error(LOG_INSTANCIA, "No se pudo crear el hilo para el Dump");
	}

	conectarCoordinador();
	PUNTERO_CIRCULAR = 0;
	PUNTERO_LRU = 0;
	PUNTERO_BSU = 0;

	//para pruebas
/*
	CANTIDAD_ENTRADAS = 10;
	TAMANIO_ENTRADAS = 10;
	TABLA_ENTRADAS = list_create();
	ALMACENAMIENTO = (char**) calloc(CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
	memset(ALMACENAMIENTO, '\0', sizeof(ALMACENAMIENTO[0][0])*CANTIDAD_ENTRADAS*TAMANIO_ENTRADAS);


	test_recibirSet("raquel", "raquelita-nos-salva-con-sus-teses-y-aspirinas\0");
	test_recibirSet("claudio", "pelado-bueno\0");
	test_recibirSet("juani", "cabezon-tonto\0");
	test_recibirSet("claudio", "pelado\0");
	test_recibirSet("juani", "cabezon-tonto-no-es-pero\0");
*/


	//test_recibirSet("perrito", "perrito-bueno");

	//reincorporar("claudio");
/*
	for(i=0; i<CANTIDAD_ENTRADAS; i++){
				printf("El valor del almacenamiento en la posicion [%d] es: %s\n", i,ALMACENAMIENTO[i]);
	}
*/
	/*
	puts("llego aca\n");
	memset(&paqueteStore, 0, sizeof(t_store));
	strcpy(paqueteStore.clave, "claudio");
	realizarStore("claudio");
*/
	//valorEnDisco("claudio");



	while(CONNECTED){ //espera que llegue una operacion
		log_trace(LOG_INSTANCIA, "Disponible, esperando nueva operacion");
		recibirOperacion();
	}

	sleep(1);
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
	paqueteSet.valor = malloc(strlen(valor)+1);
	strcpy(paqueteSet.valor, valor);
	paqueteSet.sizeValor = strlen(valor);
	printf("clave: %s\n", paqueteSet.clave);
	realizarSet(entradasNecesarias(paqueteSet.valor));
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

	log_info(LOG_INSTANCIA, "Conectada a Coordinador.");
	CONNECTED = 1;
}

void recibirOperacion(){
	t_head header = recvHead(SOCKET_COORDINADOR);
	switch(header.context){
	case INIT_INSTANCIA:
		recv(SOCKET_COORDINADOR, &paqueteInit, header.mSize,0);
		CANTIDAD_ENTRADAS = paqueteInit.cantidadEntradas;
		TAMANIO_ENTRADAS = paqueteInit.sizeofEntrada;
		TABLA_ENTRADAS = list_create();
		ALMACENAMIENTO = (char**) calloc(CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
		memset(ALMACENAMIENTO, '\0', sizeof(ALMACENAMIENTO[0][0])*CANTIDAD_ENTRADAS*TAMANIO_ENTRADAS);
		break;
	case OPERACION_SET:
		recvSet(SOCKET_COORDINADOR, &paqueteSet);
		realizarSet(entradasNecesarias(paqueteSet.valor));
		log_trace(LOG_INSTANCIA,"SET-se setio la clave: %s , con el valor: %s", paqueteSet.clave, paqueteSet.valor);
		break;
	case OPERACION_STORE:
		recv(SOCKET_COORDINADOR, &paqueteStore, header.mSize, 0);
		if (realizarStore(paqueteStore.clave)){
			log_trace(LOG_INSTANCIA,"STORE- se guardo la clave %s", paqueteStore.clave);

		}

		break;

	case REINCORPORACION_INSTANCIA:
		//recibir el sring con todas las claves a levantar de disco

		break;
	case ORDEN_COMPACTAR:
		compactar();
		break;
	case ERROR_HEAD:
		CONNECTED = 0;
		//procesa error
		break;
	default:
		puts("Se desconoce el error");
		break;
	}
}

void* atenderDump(){
	while(CONNECTED){
		sleep(INTERVALO_DUMP);
		realizarDump();
	}
	return NULL;
}

void realizarDump(){
	void realizarDump_dato(void* dato){
		char* clave = ((t_entrada*)dato)->clave;
		log_trace(LOG_INSTANCIA,"DUMP-se guarda en disco la clave: %s\n",clave);
		realizarStore(clave);
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

void liberar(char* valor){
	if(valor != NULL){
		free(valor);
	}
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
/*
	if(valor != NULL){
		free(valor);
	}*/
	//liberar(valor);
	//liberar(valorAgrabar);
	//free(valor);
	//free(valorAgrabar);

	//creo que se deberia liberar aca, pero si lo hago se me rompe el printf y no se pq
}

void realizarSet_Agregar(int entradasNecesarias){
	t_entrada* dato = malloc(sizeof(t_entrada));
	int noSeAgrego = 1;
	int posicion;

	while(noSeAgrego){
		if(hayEntradasDisponibles(entradasNecesarias)){
			puts("si, hay entradas\n");
			if(hayEspacioContiguo(entradasNecesarias, &posicion)){
				//se recibe la posicion por referencia de hayespacioscontiguos
				puts("Hay espacios contiguos\n");
				strcpy(dato->clave, paqueteSet.clave);
				dato->tamanio = paqueteSet.sizeValor;
				dato->cantidadUtilizada = entradasNecesarias;
				dato->posicion = posicion;

				list_add(TABLA_ENTRADAS, dato);
				almacenarDato(posicion, paqueteSet.valor, entradasNecesarias);
				noSeAgrego = 0;
			}else{
				puts("No hay espacios contiguos\n");
				//compactar();
				enviarOrdenDeCompactar();
			}
		}else{
			puts("Sin entradas, se corre el algoritmo\n");
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
	int cantidadReferencias;
	dato             = obtenerDato(clave);
	posicion         = dato->posicion;
	cantidadEntradas = dato->cantidadUtilizada;

	for(i=0;i<cantidadEntradas;i++){
		free(ALMACENAMIENTO[posicion+i]); // Se libera lo que estaba almacenado
		ALMACENAMIENTO[posicion+i] = NULL;
	}

	bool esElDato(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)==0){
			cantidadReferencias= ((t_entrada*)dato)->cantidadReferencias;
			return true;
		}
		return false;
	}
	list_remove_by_condition(TABLA_ENTRADAS, esElDato);
	free(dato); // Ya no se usa más así que se libera

	realizarSet_Agregar(entradasNecesarias);
	setearOperaciones(cantidadReferencias+1);
}

void realizarSet(int entradasNecesarias){
	//esta funcion agrega o modifica una entrada en la lista e entradas y
	// le envia el dato a almacenamiento
	if (yaExisteClave(paqueteSet.clave)){
		puts("ya existe\n");
		realizarSet_Actualizar(entradasNecesarias);
	}else{
		puts("no existe\n");
		realizarSet_Agregar(entradasNecesarias);
		setearOperaciones(1);
	}
}

void setearOperaciones(int cantidad){
	t_entrada* dato = obtenerDato(paqueteSet.clave);
	dato->cantidadReferencias= 1;
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
	puts("Corre el algoritmo circular");
	while(mateAuno){
		if(ALMACENAMIENTO[PUNTERO_CIRCULAR]== NULL){
			PUNTERO_CIRCULAR++;
		}else{
			dato = obtenerDato_posicion(PUNTERO_CIRCULAR);
			cantidad = dato->cantidadUtilizada;
			printf("Puntero %d , la cantidad es: %d\n",PUNTERO_CIRCULAR, cantidad);
			if (cantidad == 1){
				//es un dato atomico->lo mato
				strcpy(clave, dato->clave);
				ALMACENAMIENTO[PUNTERO_CIRCULAR]= NULL;

				bool esElDato(void* dato){
						if (strcmp(((t_entrada*)dato)->clave, clave)==0){
									return true;
						}
						return false;
					}
				list_remove_by_condition(TABLA_ENTRADAS, esElDato);

				PUNTERO_CIRCULAR++;
				mateAuno = 0;
			}else{
				PUNTERO_CIRCULAR++;
			}
		}
	}
}

void algoritmoLRU(){

}

void algoritmoBSU(){

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

void reincorporar(char* claves){
	int i=0;
	char** clavesAleer = string_split(claves, ",");

	while(clavesAleer[i] != NULL){
		char * valor = valorEnDisco(clavesAleer[i]);
		if (valor != NULL){
			test_recibirSet(clavesAleer[i], valor);
		}
		i++;
	}
	i=0;
	while(clavesAleer[i] != NULL){
		puts("se libera posicion\n");
		free(clavesAleer[i]);
		i++;
	}

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
		log_error(LOG_INSTANCIA, "REINCORPORACION- error al arbir el archivo: %s", archivo);
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

