#include "Instancia.h"
#include <string.h>

int main(int argc, char* argv[]) {
	LOG_INSTANCIA = log_create("../logs/logDeInstancia.log", "Instancia", true, LOG_LEVEL_TRACE);
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");
	cargarConfig(argv[1]);

	puts("No conectada, se presenta al coordinador");
	conectarCoordinador();
	connected = 1;

	while(connected){ //espera que llegue una operacion
		puts("Disponible, esperando que el coordinador envie una sentencia");
		recibirOperacion();
	}

	log_destroy(LOG_INSTANCIA);
	return EXIT_SUCCESS;
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
	case ERROR_HEAD:
		connected = 0;
		//procesa error
		break;
	default:
		puts("Se desconoce el error");
		break;
	}
}

int entradasNecesarias(char* valor){
	return (int) ceil(strlen(valor)/TAMANIO_ENTRADAS);
}

bool hayEntradasDisponibles(int tamanio){
	int i;
	int cantidad = 0;
	for(i=0; i< CANTIDAD_ENTRADAS; i++){
		if (ALMACENAMIENTO[i] == 0){ //encontro un 0
			cantidad++;
		}
	}
	if (cantidad >= tamanio){
		return true;
	}
	return false;
}

bool hayEspacioContiguo(int tamanio, int* posicion){\
	int i=0;
	int j;
	while(i != CANTIDAD_ENTRADAS){
		if(ALMACENAMIENTO[i] != 0){
			for(j=1; j<tamanio; j++){
				if (ALMACENAMIENTO[i] == 0){
					continue;
				}
				return true;
			}
			i=i+j;
		}else{
			i++;
		}
	}
	return false;
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
	int posicion=0;
	datoEntrada dato;
	if (yaExisteClave(paqueteSet.clave)){

	}else{
		if (hayEntradasDisponibles(entradasNecesarias)) { // hay suficientes entradas para mi nuevo dato
			if (hayEspacioContiguo(entradasNecesarias, &posicion)){ //espacio contiguo en memoria
					strcpy(dato.clave, paqueteSet.clave);
					dato.tamanio = entradasNecesarias;
					dato.cantidadReferencias= 1;
					dato.posicion = posicion;
					almacenarDato(posicion, paqueteSet.valor,entradasNecesarias);
					list_add(TABLA_ENTRADAS, &dato);
			}else{  // no hay espacio contiguo en memoria pero si cantida de espacios => compactar

			}
		}else{  //no hay suficientes entradas para almacenar -> va a reemplazar
				correrAlgoritmoDeReemplazo(dato);
		}
	}
}

void realizarStore(char clave[40]){

}

void correrAlgoritmoDeReemplazo(){

}

bool yaExisteClave(char clave[40]){
	return false;
}

