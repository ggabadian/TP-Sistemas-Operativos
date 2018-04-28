//se usan dos sockets,uno para quedarse escuchando y otro para contestar
#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <string.h>
#include <commons/log.h>

int openSocket() {
	int s = socket(PF_INET, SOCK_STREAM, 0);
	if (s == -1) {
		puts("error al abrir socket");
	}
	return s;
}
void bindToPort(int socket, int port) {
	struct sockaddr_in nombre;
	nombre.sin_family = PF_INET;
	nombre.sin_port = (in_port_t) htons(port);
	nombre.sin_addr.s_addr = htonl(INADDR_ANY);

	int reus = 1;
	if (setsockopt(socket, SOL_SOCKET, SO_REUSEADDR, (char*) &reus, sizeof(int))
			== -1) {
		perror("no es posible reusar el socket");
	}
	int c = bind(socket, (struct sockaddr*) &nombre, sizeof(nombre));
	if (c == -1) {
		perror("no se puede enlazar al puerto: direccion ya esta en uso\n");
	}
}

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

			switch(opcion)
			{
				case 1:
					system("clear");
					puts("PAUSAR / CONTINUAR");
					printf("Oprima 'P' para Pausar o 'C' para Continuar: ");
					scanf("%d", &opcion);
					if(opcion == 'P')
						printf("\n\nSe eligió Pausar");
					if(opcion == 'C')
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
					puts("Los deadloscks existentes son:");
				break;
			}
}

int main() {
//	const int PORT = 5000;
//	puts("soy planificador");
//	char* mensaje = "hola cliente";
//	int listener = openSocket();
//
//	if (listener == -1) {
//		puts("error en listener");
//		return 1;
//	}
//
//	bindToPort(listener, PORT);
//
//	if (listen(listener, 10) == -1) { //10 es el numero máximo de clientes que puede escuchar este socket
//		printf("no es posible escuchar en ese puerto\n");
//		return 1;
//	}
//	printf("enlazado al puerto\n");
//
//	while (1) {
//		struct sockaddr_storage cliente;
//		unsigned int addres_size = sizeof(cliente);
//		printf("esperando al cliente\n");
//		int connect = accept(listener, (struct sockaddr*) &cliente,
//				&addres_size); //syscall bloqueante
//		if (connect == -1) {
//			printf("no se puede conectar socket secundario\n");
//		}
//		printf("Atendiendo al cliente\n");
//		//send(connect,mensaje,strlen(mensaje),0);
//		mensaje = NULL;
//
//		char* buffer = malloc(16); //buffer donde voy a almacenar lo que reciba
//		int bytesRecibidos = recv(connect,(void*) buffer, 4, 0); //MSG_WAITALL
//		if (bytesRecibidos <= 0) { //devuelve negativo si hay un error del otro lado de la conexion
//			perror("el cliente se desconecto");
//			return 1;
//		}
//		printf("me llegaron %d bytes con %s", bytesRecibidos, buffer);
//		free(buffer);
//
//		close(connect);
//	}

	consola();

	return 0;
}
