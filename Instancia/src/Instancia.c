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
		puts("Se recibio el init");
	break;
	case OPERACION_SET:
		recvSet(SOCKET_COORDINADOR, &paqueteSet);
		puts("Se recibio un set");
		realizarSet();

	break;
	case OPERACION_STORE:
	//	recv(SOCKET_COORDINADOR,&paqueteStore,header.mSize,0);
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

void realizarSet(){

}
