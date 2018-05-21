#include "ESI.h"

int main(void) {
	puts("Hola soy el ESI");

	int planificadorSocket = connectSocket(IP, PUERTO);

	int enviar = 1;
	char message[PACKAGESIZE];

	printf(
			"Conectado al planificador. Bienvenido al sistema, ya puede enviar mensajes. Escriba 'exit' para salir\n");

	while (enviar) {
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message, "exit\n"))
			enviar = 0;
		if (enviar)
			send(planificadorSocket, message, strlen(message) + 1, 0);
	}

	close(planificadorSocket);
	return 0;

	return 0;
}
