#include "ESI.h"

int main(void) {
	puts("Hola soy el ESI");

	struct addrinfo hints;
	struct addrinfo *planificadorInfo;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(IP, PUERTO, &hints, &planificadorInfo);

	int planificadorSocket;
	planificadorSocket = socket(planificadorInfo->ai_family, planificadorInfo->ai_socktype,planificadorInfo->ai_protocol);

	connect(planificadorSocket, planificadorInfo->ai_addr, planificadorInfo->ai_addrlen);
	freeaddrinfo(planificadorInfo);

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
