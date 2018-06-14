#include "Coordinador.h"
#include <parsi/parser.h>

int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 32);

	puts("Esperando cliente.");

	while (1) {
		int identificador;

		int socketCliente = acceptSocket(listeningSocket);

		identificador = recibirHead(socketCliente);
		if (identificador == ERROR_HEAD) {
			puts(
					"Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
			//(Pendiente) log error
		} else {
			printf("Conectado a %s.\n", identificar(identificador));
		}

		crearThread(identificador, socketCliente);
	}

	close(listeningSocket);
	return 0;
}

//(Pendiente) BUG - Valgrind dice que podria estar perdiendo memoria
//(Pendiente) BUG - Valgrind dice que hay problemas al enviar estructuras con send()

void crearThread(int id, int socket) {
	pthread_t thread;

	if (id == PLANIFICADOR) {
		int statusPlanificador = 1;
		int socketPlanificador = socket;

		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador,
				(void*) &socketPlanificador);
		if (statusPlanificador) {
			puts("Error en la creación de thread para Planificador");
			//(Pendiente) log error
		}
	} else if (id == ESI) {
		int statusESI = 1;
		int *socketESI = malloc(sizeof(int)); // (PENDIENTE) Hacer free
		socketESI = &socket;

		statusESI = pthread_create(&thread, NULL, &threadESI, (void*) socketESI);

		if (statusESI) {
			puts("Error en la creación de thread para ESI");
			//(Pendiente) log error
		}
	} else if (id == INSTANCIA) {
		int statusInstancia = 1;
		int socketInstancia = socket;

		statusInstancia = pthread_create(&thread, NULL, &threadInstancia,
				(void*) &socketInstancia);
		if (statusInstancia) {
			puts("Error en la creación de thread para Instancia");
			//(Pendiente) log error
		}
	} else {
		puts("Error al crear thread: La conexión es desconocida");
		//(Pendiente) log error
	}
}

void* threadPlanificador(void* socket) {
	int* socketPlanificador = (int*) socket;

	while (1) {
//		int headPlanificador = recibirHead(ePlanificador->socketPlanificador);
//		hacerAlgo(headPlanificador);
	}
//	recibirMensaje(ePlanificador->socketPlanificador);

	close(*socketPlanificador);
	free(socketPlanificador);

	return NULL;
}

void* threadESI(void* socket) {
	int* socketESI = (int*) socket;

	while (1) {
//		int headESI = recibirHead(eESI->socketESI);
//		hacerAlgo(headESI);

		t_esi_operacion parsed;

		recv(*socketESI,&parsed,sizeof(t_esi_operacion),0);

		if (parsed.valido) {
			switch (parsed.keyword) {
			case GET:
				printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
				break;
			case SET:
				printf("SET\tclave: <%s>\tvalor: <%s>\n",
						parsed.argumentos.SET.clave,
						parsed.argumentos.SET.valor);
				break;
			case STORE:
				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
				break;
			default:
				fprintf(stderr, "No pude interpretar \n");
				exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		} else {
			fprintf(stderr, "La linea no es valida\n");
			exit(EXIT_FAILURE);
		}
	}

//	recibirMensaje(eESI->socketESI);

	//close(*socketESI);
	//free(socketESI);
	return NULL;
}

void* threadInstancia(void* socket) {
	int* socketInstancia = (int*) socket;

	sendInitInstancia(*socketInstancia, CANTIDAD_ENTRADAS,
			BYTES_ENTRADA);

	while (1) {
//		int headInstancia = recibirHead(eInstancia->socketInstancia);
//		hacerAlgo(headInstancia);
	}

//	recibirMensaje(eInstancia->socketInstancia);

	close(*socketInstancia);
	free(socketInstancia);
	return NULL;
}

void recibirMensaje(int socket) {
	char *package = malloc(1024);
	while (1) {
		recv(socket, (void*) package, 1024, 0);
		printf("%s", package);
	}
	close(socket);
	free(package);
}

void sendInitInstancia(int socket, int cantEntradas, int sizeEntrada) {

	t_InitInstancia paquete;

	paquete.cantidadEntradas = cantEntradas;
	paquete.sizeofEntrada = sizeEntrada;

	// Envia el HEAD para que la instancia sepa lo que va a recibir
	enviarHead(socket, initDatosInstancia);

//(PENDIENTE) Serializacion de paquete

//Envia el paquete
	send(socket, (void*) &paquete, sizeof(paquete), 0);

	puts("Configuración inicial enviada a Instancia.");
}

void freePackage(char **package) {
	free(*package);
}
