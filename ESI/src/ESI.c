#include "ESI.h"

int main(void) {
	puts("Iniciando ESI...");
	cargarConfig();

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);
	int planificadorSocket = connectSocket(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);

	int enviar = 1;
	char message[PACKAGESIZE];

	if (coordinadorSocket >= 0){
		send(coordinadorSocket, ESI, 4, 0); // Le avisa que es un ESI
		printf("Conectado a Coordinador. Escriba 'exit' para salir\n");
	}

	if (planificadorSocket >= 0){
		printf("Conectado a Planificador. \n");
		send(planificadorSocket, "Hola Planificador", 18, 0);
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
	close(planificadorSocket);
	return 0;

	return 0;
}
