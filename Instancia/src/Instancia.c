#include "Instancia.h"

int main(int argc, char* argv[]) {
	int i=0;
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

	//para pruebas
	CANTIDAD_ENTRADAS = 10;
	TAMANIO_ENTRADAS = 10;
	TABLA_ENTRADAS = list_create();
	ALMACENAMIENTO = (char**) calloc(CANTIDAD_ENTRADAS, TAMANIO_ENTRADAS);
	recibirSet();
	while(ALMACENAMIENTO[i] != NULL){
		printf("El valor del almacenamiento en la posicion [%d] es: %s", i, ALMACENAMIENTO[i]);
	}
	/*
	while(CONNECTED){ //espera que llegue una operacion
		puts("Disponible, esperando que el coordinador envie una sentencia");
		recibirOperacion();
	}*/

	log_destroy(LOG_INSTANCIA);
	free(ALMACENAMIENTO);
	return EXIT_SUCCESS;
}

void recibirSet(){
	memset(&paqueteSet, 0, sizeof(t_set));
	strcpy(paqueteSet.clave, "claudio");
	paqueteSet.valor = malloc(strlen("pelado")+1);
	strcpy(paqueteSet.valor, "pelado");
	paqueteSet.sizeValor = strlen("pelado");
	printf("el valor es: %s",paqueteSet.valor );
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
	return (int) ceil(strlen(valor)/TAMANIO_ENTRADAS);
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

bool hayEspacioContiguo(int tamanio, int* posicion){
	posicion = 0;
	return true;
}

void almacenarDato(int posicion, char* valor, int cantidadEntradas){
	int len = strlen(valor);
	int i;
	int inicio=0;
	int desplazamiento= TAMANIO_ENTRADAS;
	char* subvalor;

	for(i=0; i<cantidadEntradas; i++){
		if(len> TAMANIO_ENTRADAS){
			subvalor = string_substring(valor, inicio, desplazamiento);
			memcpy(ALMACENAMIENTO[posicion=i],subvalor, desplazamiento);
			inicio = inicio+desplazamiento;
		}else{
			subvalor = string_substring(valor, inicio+(i*desplazamiento), len);
			memcpy(ALMACENAMIENTO[posicion=i],subvalor, desplazamiento);
		}
		len = len-desplazamiento;
	}

}

void realizarSet(int entradasNecesarias){
	int seAlmaceno = 1;
	int posicion=0;
	int cantidadABorrar=0;
	int i;
	t_entrada* dato;
	if (yaExisteClave(paqueteSet.clave)){
		//dato = obtenerDato(paqueteSet.clave);
		//ocupa el mismo espacio
		if (entradasNecesarias == dato->cantidadUtilizada){
			dato->tamanio = paqueteSet.sizeValor;
			almacenarDato(dato->posicion, paqueteSet.valor, entradasNecesarias);
		}
		//ocupa mas espacio
		if (entradasNecesarias > dato->cantidadUtilizada){

		}
		//ocupa menos espacio
		if (entradasNecesarias < dato->cantidadUtilizada){
			cantidadABorrar = dato->cantidadUtilizada - entradasNecesarias;
			dato->cantidadUtilizada = entradasNecesarias;
			dato->tamanio = paqueteSet.sizeValor;
			almacenarDato(dato->posicion, paqueteSet.valor, entradasNecesarias);
			for(i=1;i <= cantidadABorrar;i++){
				limpiarPosicion(dato->posicion+entradasNecesarias-i);
			}
		}
	}else{ //valor nuevo
		memset(&dato,0,sizeof(t_entrada));
		while(seAlmaceno){
			if (hayEntradasDisponibles(entradasNecesarias)) { // hay suficientes entradas para mi nuevo dato
				if (hayEspacioContiguo(entradasNecesarias, &posicion)){ //espacio contiguo en memoria
						strcpy(dato->clave, paqueteSet.clave);
						dato->tamanio = entradasNecesarias;
						dato->posicion = posicion;
						almacenarDato(posicion, paqueteSet.valor,entradasNecesarias);
						list_add(TABLA_ENTRADAS, &dato);
						seAlmaceno = 0;
				}else{  // no hay espacio contiguo en memoria pero si cantida de espacios => compactar
					enviarOrdenDeCompactar();
				}
			}else{  //no hay suficientes entradas para almacenar -> va a reemplazar
					correrAlgoritmoDeReemplazo(dato);
			}
		}
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

}
void compactar(){

}

void limpiarPosicion(int posicion){
	ALMACENAMIENTO[posicion] = NULL;
}

bool yaExisteClave(char* clave){
	bool claveBuscada(void* dato){
		if (strcmp(((t_entrada*)dato)->clave, clave)){
			return true;
		}
		return false;
	}
	return list_any_satisfy(TABLA_ENTRADAS, claveBuscada);
}




