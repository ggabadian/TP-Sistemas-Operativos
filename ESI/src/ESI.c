#include "ESI.h"

int id; //Este es el ID del ESI. Lo provee el Planificador en el handshake

int main(void) {
	puts("Iniciando ESI...");
	cargarConfig();

/*
	1. Conectarse al planificador.
	2. Handshake planificador (recibir ID)
	3. Conectarse al Coordinador
	4. Handshake Coordinador
	5. Esperar orden de ejecucion del planificador
	6. Parsear instruccion
	7. Encviar al coordinador la orden de ejecucion
	8. Recibir resultado por parte del coordinador el resultado
	9. Transimitir resultado al p,anificador
	10. Volver a 5.
*/

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);
	printf("Conectado a Coordinador. \n");
	send(coordinadorSocket, ESI, 4, 0); // Le avisa que es un ESI

	int planificadorSocket = connectSocket(IP_PLANIFICADOR, PUERTO_PLANIFICADOR);
	printf("Conectado a Planificador. \n");

	int enviar = 1;
	char message[PACKAGESIZE];

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
