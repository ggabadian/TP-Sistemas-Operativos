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
	puts("Iniciando planificador...");
	cargarConfigPlanificador();

	t_head header;
	header.context = PLANIFICADOR;
	header.mSize = 0;

//	printf("PUERTO= %s\n", PUERTO);
//	printf("ALGORITMO= %s\n", ALGORITMO);
//	printf("ALFA= %d\n", ALFA);
//	printf("ESTIMACION= %d\n", ESTIMACION_I);
//	printf("IP COORDINADOR= %s\n", IP_COORDINADOR);
//	printf("PUERTO COORDINADOR= %d\n", PUERTO_COORDINADOR);
//	printf("CLAVES BLOQUEADAS= %s", CL_BLOQUEADAS[0]);

	int listeningSocket = listenSocket(PUERTO);

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);
	printf("Conectado a Coordinador. \n");
	sendHead(coordinadorSocket, header); // Le avisa que es el planificador

	listen(listeningSocket, BACKLOG);

	t_head identificador;
	int status = 1;

	int idESI = 0; // Cantidad de ESIs conectados

	int socketCliente = acceptSocket(listeningSocket);

	identificador = recvHead(socketCliente);
	if (identificador.context == ERROR_HEAD) {
		puts("Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
	} else {
		printf("Conectado a %s.\n", identificar(identificador.context));
	}

	if (identificador.context==ESI) { // Si es un ESI, le asigna un id
		idESI ++;
		send(socketCliente, &idESI, sizeof(idESI), 0);
	}

	char package[PACKAGESIZE];

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
