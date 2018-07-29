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
	/*
	CANTIDAD_ENTRADAS = 10;
	TAMANIO_ENTRADAS = 10;
	TABLA_ENTRADAS = list_create();
	ALMACENAMIENTO = (char**) calloc(CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);

	test_recibirSet("raquel", "raquelita-nos-salva-con-sus-teses-y-aspirinas");
	test_recibirSet("claudio", "pelado-bueno");
	test_recibirSet("juani", "cabezon-tonto");
	test_recibirSet("claudio", "pelado");
	test_recibirSet("juani", "cabezon-tonto-no-es-pero");

	for(i=0; i<CANTIDAD_ENTRADAS; i++){
			printf("El valor del almacenamiento en la posicion [%d] es: %s\n", i,ALMACENAMIENTO[i]);
	}

	test_recibirSet("perrito", "perrito-bueno");
	*/

	while(CONNECTED){ //espera que llegue una operacion
		puts("Disponible, esperando que el coordinador envie una sentencia");
		recibirOperacion();
	}

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
	free(paqueteSet.valor);
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
		puts("Se recibio el init");
		break;
	case OPERACION_SET:
		recvSet(SOCKET_COORDINADOR, &paqueteSet);
		printf("test: %s\n", paqueteSet.valor);
		puts("Se recibio un set");
		realizarSet(entradasNecesarias(paqueteSet.valor));
		break;
	case OPERACION_STORE:
		recv(SOCKET_COORDINADOR, &paqueteStore, header.mSize, 0);
		realizarStore(paqueteStore.clave);
		puts("Se recibio un store");
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
	printf("Cantidad entradas: %d\n", cantidad);
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
		free(valorAgrabar);
		++nEntrada;
	}
}

void realizarSet_Agregar(int entradasNecesarias){
	int i;
	t_entrada* dato = malloc(sizeof(t_entrada));
	//int noSeAgrego = 1;
	int posicion;

	//while(noSeAgrego){
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
				//noSeAgrego = 0;
			}else{
				puts("No hay espacios contiguos\n");
				enviarOrdenDeCompactar();
			}
		}else{
			puts("Sin entradas, se corre el algoritmo\n");
			correrAlgoritmoDeReemplazo();

			for(i=0; i<CANTIDAD_ENTRADAS; i++){
				printf("El valor del almacenamiento en la posicion [%d] es: %s\n", i,ALMACENAMIENTO[i]);
			}
		}
	//}
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


