#include "Coordinador.h"
#include "configCoordinador.c"

int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	int listeningSocket = newSocket();

	listen(listeningSocket, BACKLOG);

	puts("Esperando cliente.");

	struct sockaddr_in addr;
	socklen_t addrlen = sizeof(addr);

	int socketCliente = accept(listeningSocket, (struct sockaddr *) &addr, &addrlen);

	char package[PACKAGESIZE];
	int status = 1;

	// (Pendiente) Discriminacion de clientes
	printf("Cliente conectado, esperando mensaje.\n");

	while (status != 0) {
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if (status != 0) printf("%s", package);
	}

	close(socketCliente);
	close(listeningSocket);

	return 0;
}
