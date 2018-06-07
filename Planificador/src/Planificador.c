#include "Planificador.h"

void consola() {
	system("clear");
	puts("CONSOLA PLANIFICADOR, DIGITE EL Nro DE COMANDO A EJECUTAR:\n");
	puts("1) Pausar / Continuar");
	puts("2) Bloquear (Clave, ID)");
	puts("3) Desbloquear (Clave)");
	puts("4) Listar (Recurso)");
	puts("5) Kill (ID)");
	puts("6) Status (Clave)");
	puts("7) Deadlock");
	printf("Ingrese Nro de comando: ");
	int opcion;
	char* clave;
	char* id;
	char* recurso;
	scanf("%d", &opcion);

	switch (opcion) {
	case 1:
		system("clear");
		puts("PAUSAR / CONTINUAR");
		printf("Oprima 'P' para Pausar o 'C' para Continuar: ");
		scanf("%d", &opcion);
		if (opcion == 'P')
			printf("\n\nSe eligió Pausar");
		if (opcion == 'C')
			printf("\n\nSe eleigió Continuar");
		else
			printf("\n\nOpcion incorrecta");
		break;

	case 2:
		system("clear");
		puts("BLOQUEAR");
		printf("Inserte Clave: ");
		scanf("%s", clave);
		printf("\nInserte ID: ");
		scanf("%s", id);
		break;

	case 3:
		system("clear");
		puts("DESBLOQUEAR");
		printf("Inserte Clave: ");
		scanf("%s", clave);
		break;

	case 4:
		system("clear");
		puts("LISTAR");
		printf("Inserte Recurso: ");
		scanf("%s", recurso);
		break;

	case 5:
		system("clear");
		puts("KILL");
		printf("Escriba el ID del proceso a matar: ");
		scanf("%s", id);
		break;

	case 6:
		system("clear");
		puts("STATUS");
		printf("El algoritmo que está corriendo es: ");
		break;

	case 7:
		system("clear");
		puts("Los deadlocks existentes son:");
		break;
	}
}

int main() {
	puts("Hola soy el planificador");
	cargarConfigPlanificador();
//	printf("PUERTO= %s\n", PUERTO);
//	printf("ALGORITMO= %s\n", ALGORITMO);
//	printf("ALFA= %d\n", ALFA);
//	printf("ESTIMACION= %d\n", ESTIMACION_I);
//	printf("IP COORDINADOR= %s\n", IP_COORDINADOR);
//	printf("PUERTO COORDINADOR= %d\n", PUERTO_COORDINADOR);
//	printf("CLAVES BLOQUEADAS= %s", CL_BLOQUEADAS[0]);

	int listeningSocket = listenSocket(PUERTO);
	listen(listeningSocket, BACKLOG);

	int socketCliente = acceptSocket(listeningSocket);

	char package[PACKAGESIZE];
	int status = 1;

	printf("Cliente conectado. Esperando mensajes:\n");

	while (status != 0) {
		status = recv(socketCliente, (void*) package, PACKAGESIZE, 0);
		if (status != 0)
			printf("%s", package);

	}

	close(socketCliente);
	close(listeningSocket);

//	consola();

	return 0;
}
