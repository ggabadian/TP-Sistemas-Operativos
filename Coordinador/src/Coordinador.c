#include "Coordinador.h"
#include "configCoordinador.h"

struct stESI {
	int socketESI;
//	otras cosas
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

		pthread_t thread;

		int socketCliente = acceptSocket(listeningSocket);

		status= recv(socketCliente, &identificador, 4, 0);
		if (status != 0) {
			printf("Conectado a %s.\n", identificar(identificador));
		} else {
			puts("Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
			//(Pendiente) log error
		}
		if(identificador == ESI){
			int statusESI = 1;
			struct stESI estESI;
			estESI.socketESI = socketCliente;

			statusESI = pthread_create(&thread, NULL, &threadESI, (void*) &estESI);
		    if(statusESI){
		    	puts("Error en la creación de thread para ESI");
		    	//(Pendiente) log error
		    }
		}
	}

	close(listeningSocket);
	return 0;
}

//(Pendiente) BUG - Valgrind dice que podria estar perdiendo memoria

void threadESI(void* estructura){
	struct stESI* estructuraESI = (struct stESI*) estructura;

	recibirMensaje(estructuraESI->socketESI);

	free(estructuraESI);
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
