#include "Coordinador.h"

int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 5);

	puts("Esperando cliente.");

	int socketCliente = acceptSocket(listeningSocket);

	char package[1024];
	int status = 1;

	// (Pendiente) Discriminacion de clientes
	printf("Cliente conectado, esperando mensaje.\n");

	while (status != 0) {
		status = recv(socketCliente, (void*) package, 1024, 0);
		if (status != 0) printf("%s", package);
	}

	close(socketCliente);
	close(listeningSocket);

	return 0;
}
