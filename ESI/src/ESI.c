#include "ESI.h"

int main(void) {
	puts("Hola soy el ESI");

	int coordinadorSocket = connectSocket(IP, PUERTO_COORDINADOR);

	int enviar = 1;
	char message[PACKAGESIZE];

	if (coordinadorSocket >= 0){
		send(coordinadorSocket, ESI, 4, 0); // Le avisa que es un ESI
		printf("Conectado a Coordinador. Escriba 'exit' para salir\n");
	}

	//(Pendiente) Conexion al planificador

	while (enviar) {
		fgets(message, PACKAGESIZE, stdin);
		if (!strcmp(message, "exit\n"))
			enviar = 0;
		if (enviar)
			send(coordinadorSocket, message, strlen(message) + 1, 0);
	}

	close(coordinadorSocket);
	return 0;

	return 0;
}
