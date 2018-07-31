#include "Instancia.h"

int main(int argc, char* argv[]) {
	//int i=0;
	//int statusThread;
	//pthread_t threadDump;

	LOG_INSTANCIA = log_create("../logs/logDeInstancia.log", "Instancia", true, LOG_LEVEL_TRACE);
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");

	cargarConfig(argv[1]);

	//se crea el hilo para atender el dump
	/*
	statusThread = pthread_create(&threadDump, NULL, &atenderDump, NULL);
	if (statusThread){
		log_error(LOG_INSTANCIA, "No se pudo crear el hilo para el Dump");
	}
	*/
	conectarCoordinador();
	PUNTERO_CIRCULAR = 0;
	PUNTERO_LRU = 0;
	PUNTERO_BSU = 0;

	//para pruebas

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


	//test_recibirSet("perrito", "perrito-bueno");
	/*
	for(i=0; i<CANTIDAD_ENTRADAS; i++){
				printf("El valor del almacenamiento en la posicion [%d] es: %s\n", i,ALMACENAMIENTO[i]);
	}*/

	memset(&paqueteStore, 0, sizeof(t_store));
	strcpy(paqueteStore.clave, "juani");
	realizarStore("juani");


/*
	while(CONNECTED){ //espera que llegue una operacion
		puts("Disponible, esperando que el coordinador envie una sentencia");
		recibirOperacion();
	}*/

	log_destroy(LOG_INSTANCIA);
	free(ALMACENAMIENTO);
	return EXIT_SUCCESS;
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
		break;
	case OPERACION_STORE:
		recv(SOCKET_COORDINADOR, &paqueteStore, header.mSize, 0);
		realizarStore(paqueteStore.clave);
		break;
	case OPERACION_COMPACTAR:
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

void atenderDump(){
	while(CONNECTED){
		sleep(INTERVALO_DUMP);
		realizarDump();
	}
}

void realizarDump(){

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
		nEntrada++;
	}
	//free(valorAgrabar);
	//creo que se deberia liberar aca, pero si lo hago se me rompe el printf y no se pq
}

void realizarSet_Agregar(int entradasNecesarias){
	int i;
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
				compactar();
				//enviarOrdenDeCompactar();
			}
		}else{
			puts("Sin entradas, se corre el algoritmo\n");
			correrAlgoritmoDeReemplazo();

			for(i=0; i<CANTIDAD_ENTRADAS; i++){
				printf("El valor del almacenamiento en la posicion [%d] es: %s\n", i,ALMACENAMIENTO[i]);
			}
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
		ALMACENAMIENTO[posicion+i] = NULL;
	}

	bool esElDato(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)==0){
					return true;
		}
		return false;
	}
	list_remove_by_condition(TABLA_ENTRADAS, esElDato);

	realizarSet_Agregar(entradasNecesarias);
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
	}
}

void enviarOrdenDeCompactar(){
	/*t_head header;
	char* mensaje = "Hay que compactar";
	header.context = necesidadCompactar;
	header.mSize = strlen(mensaje);
	sendHead(SOCKET_COORDINADOR, header);
	send(SOCKET_COORDINADOR, mensaje, strlen(mensaje), 0);*/
}

void realizarStore(char* clave){
	char* valor = obtenerValor(clave);
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
	if (lseek(fd, tamanio - 1, SEEK_SET) == -1) { close(fd);
	}else{
	// Prueba permiso de escritura... escribiendo :)
	if (write(fd, " ", 1) == -1) { close(fd);
    }else{
   	// Ahora mapeamos (asociamos) la memoria con el espacio en disco
   	char* map = mmap(0, tamanio, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_SHARED, fd, 0);
   	if (map == MAP_FAILED) { close(fd);
    }else{

    strcpy(map, valor);

	if (msync(map, tamanio, MS_SYNC) == -1) {
		puts("no se sincronizo");
	}

	if (munmap(map, tamanio) == -1) {
		close(fd);
	}

	close(fd);
	free(archivo);
    }}}
}

char* obtenerValor(char* clave){
	int i;
	char* valor= string_new();
	t_entrada* dato= obtenerDato(clave);

	for(i=0;i<dato->cantidadUtilizada;i++){
		string_append(&valor,ALMACENAMIENTO[dato->posicion+i]);
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


