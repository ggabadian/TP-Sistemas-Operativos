#include "Coordinador.h"
#include "configCoordinador.h"

struct stPlanificador {
	int socketPlanificador;
//	otras cosas
};

struct stESI {
	int socketESI;
//	otras cosas
};

struct stInstancia {
	int socketInstancia;
	int cantidadEntradas;
	int sizeofEntrada;
};

int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 32);

	puts("Esperando cliente.");

	while(1){
		int status = 0;
		int identificador;

		int socketCliente = acceptSocket(listeningSocket);

		status= recv(socketCliente, &identificador, 4, 0);
		if (status != 0) {
			printf("Conectado a %s.\n", identificar(identificador));
		} else {
			puts("Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
			//(Pendiente) log error
		}

		crearThread(identificador, socketCliente);
	}

	close(listeningSocket);
	return 0;
}

//(Pendiente) BUG - Valgrind dice que podria estar perdiendo memoria
//(Pendiente) BUG - Warning en status
//(Pendiente) BUG - Si se cierra una entidad (que estaba conectada al Coordinador),
//					el Coordinador se rompe

void crearThread(int id, int socket){
	pthread_t thread;

	if (id == PLANIFICADOR){
		int statusPlanificador = 1;
		struct stPlanificador estPlanificador;
		estPlanificador.socketPlanificador = socket;

		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador, (void*) &estPlanificador);
		if(statusPlanificador){
			puts("Error en la creación de thread para Planificador");
			//(Pendiente) log error
		}
	}
	else if (id == ESI){
		int statusESI = 1;
		struct stESI estESI;
		estESI.socketESI = socket;

		statusESI = pthread_create(&thread, NULL, &threadESI, (void*) &estESI);
		if(statusESI){
			puts("Error en la creación de thread para ESI");
			//(Pendiente) log error
		}
	}
	else if (id == INSTANCIA){
		int statusInstancia = 1;
		struct stInstancia estInstancia;
		estInstancia.socketInstancia = socket;
		estInstancia.cantidadEntradas = 25;
		estInstancia.sizeofEntrada = 10;

		statusInstancia = pthread_create(&thread, NULL, &threadInstancia, (void*) &estInstancia);
		if(statusInstancia){
			puts("Error en la creación de thread para Instancia");
			//(Pendiente) log error
		}
	}
	else {
		puts("Error al crear thread: La conexión es desconocida");
		//(Pendiente) log error
	}
}

void threadPlanificador(void* estructura){
	struct stPlanificador* ePlanificador = (struct stPlanificador*) estructura;

	recibirMensaje(ePlanificador->socketPlanificador);

	free(ePlanificador);
}

void threadESI(void* estructura){
	struct stESI* eESI = (struct stESI*) estructura;

	recibirMensaje(eESI->socketESI);

	free(eESI);
}

void threadInstancia(void* estructura){
	struct stInstancia* eInstancia = (struct stInstancia*) estructura;

	sendInitInstancia(eInstancia->socketInstancia, eInstancia->cantidadEntradas, eInstancia->sizeofEntrada);

	recibirMensaje(eInstancia->socketInstancia);

	free(eInstancia);
}

void recibirMensaje(int socket){
	char *package = malloc(1024);
	while (1) {
		recv(socket, (void*) package, 1024, 0);
		printf("%s", package);
	}
	close(socket);
	free(package);
}

void sendInitInstancia(int socket, int cantEntradas, int sizeEntrada){
	struct estMensaje {
		int cantidadEntradas;
		int sizeofEntrada;
	} paquete;

	paquete.cantidadEntradas = cantEntradas;
	paquete.sizeofEntrada = sizeEntrada;

	// Envia el HEAD para que la instancia sepa lo que va a recibir
	send(socket, &initDatosInstancia, 4, 0); // Por PROTOCOLO;

	//Envia el paquete
	send(socket, (void*) &paquete, sizeof(paquete), 0);

	puts("Configuración inicial enviada a Instancia.");
}
