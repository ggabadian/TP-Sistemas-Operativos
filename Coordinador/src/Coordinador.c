#include "Coordinador.h"

int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 32);

	puts("Esperando cliente.");

	int status = 0;
	int identificador; // Por PROTOCOLO

	int socketCliente = acceptSocket(listeningSocket);

	status= recv(socketCliente, &identificador, 4, 0);
	if (status != 0) {
		printf("Conectado a %s.\n", identificar(identificador));
	} else {
		puts("Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
		//(Pendiente) log error
	}

	char package[1024];

	//(Pendiente) Hilos

	while (status != 0) {
		status = recv(socketCliente, (void*) package, 1024, 0);
		if (status != 0) printf("%s", package);
	}

	close(socketCliente);
	close(listeningSocket);

	return 0;
}
