#include "ESI.h"

int id = 0; //Este es el ID del ESI. Lo provee el Planificador en el handshake

int main(int argc, char** argv) {
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
	 8. Recibir resultado por parte del coordinador
	 9. Transimitir resultado al planificador
	 10. Volver a 5.
	 */

	// 1,2. Conectarse al planificador y hacer handshake
	int planificadorSocket=conectarAlPlanificador();
	if (planificadorSocket<0){
		return planificadorSocket; //Finaliza el ESI
	}

	// 3,4. Conectarse al coordinador y hacer handshake
	int coordinadorSocket=conectarAlCoordinador();
	if (coordinadorSocket<0){
		return coordinadorSocket; //Finaliza el ESI
	}

	printf("Presione ENTER para finalizar el ESI\n");
	char enter = 0;
	while (enter != '\r' && enter != '\n') { enter = getchar(); }

	close(coordinadorSocket);
	close(planificadorSocket);

	return 0;
}

int conectarAlPlanificador(){
	int planificadorSocket = connectSocket(IP_PLANIFICADOR,PUERTO_PLANIFICADOR);
	if (planificadorSocket < 0) {
		printf("Error al conectar al Planificador. \n");
		return planificadorSocket;
	} else {
		printf("Conectado a Planificador. \n");
	}
	send(planificadorSocket, &ESI, 4, 0);// Le avisa que es un ESI
	int status = recv(planificadorSocket, &id, sizeof(id), 0); // Recibe el id asignado por el Planificador
	if (status < 0) {
		puts("ERROR: El planificador no pudo asignar un id.\n");
		return status;
	} else {
		printf("Este es el ESI %d.\n", id);
	}
	return planificadorSocket;
}

int conectarAlCoordinador(){
	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);

	if (coordinadorSocket < 0) {
		printf("Error al conectar al Coordinador. \n");
		return coordinadorSocket;
	} else {
		printf("Conectado a Coordinador. \n");
	}
	send(coordinadorSocket, &ESI, 4, 0); // Le avisa que es un ESI
	return coordinadorSocket;
}
