#include "Planificador.h"


// Defino los nombres de las listas a utilizar
t_list *listos;
t_list *bloqueados;
t_list *finalizados;
t_list *clBloqueadas;


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
	//char* clave;
	//char* id;
	//char* recurso;
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
		//scanf("%s", clave);
		printf("\nInserte ID: ");
		//scanf("%s", id);
		break;

	case 3:
		system("clear");
		puts("DESBLOQUEAR");
		printf("Inserte Clave: ");
		//scanf("%s", clave);
		break;

	case 4:
		system("clear");
		puts("LISTAR");
		printf("Inserte Recurso: ");
		//scanf("%s", recurso);
		break;

	case 5:
		system("clear");
		puts("KILL");
		printf("Escriba el ID del proceso a matar: ");
		//scanf("%s", id);
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
	t_log* logPlanificador;//puede que haya que ponerla global si se usa en alguna funcion
	//creo el logger
	logPlanificador = log_create("../log/logDePlanificador.log", "Planificador", true, LOG_LEVEL_TRACE);
	//se usa para escribir en el archivo de log y lo muestra por pantalla
	log_trace(logPlanificador, "Iniciando Planificador");
	cargarConfigPlanificador();

	listos = list_create();
	bloqueados = list_create();
	finalizados = list_create();
	clBloqueadas = list_create();

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

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);
	printf("Conectado a Coordinador. \n");
	sendHead(coordinadorSocket, header); // Le avisa que es el planificador

	int listeningSocket = listenSocket(PUERTO);
	listen(listeningSocket, BACKLOG);

	int idESI = 0; // Cantidad de ESIs conectados

	fd_set master; 		// master file descriptor list
	fd_set read_fds; 	// temp file descriptor list for select()
	int fdmax;
	FD_ZERO(&master);	// clear the master and temp sets
	FD_ZERO(&read_fds);

	// add the listener to the master set
	FD_SET(listeningSocket, &master);
	FD_SET(coordinadorSocket, &master);

	// keep track of the biggest file descriptor
	fdmax = (
			listeningSocket > coordinadorSocket ?
					listeningSocket : coordinadorSocket);

	t_get paqueteGet;
	t_set paqueteSet;
	t_store paqueteStore;

	// main loop
	for (;;) {
		read_fds = master; // copy it
		if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
			perror("select");
			exit(4);
		}

		// run through the existing connections looking for data to read
		for (int i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &read_fds)) { // we got one!!
				if (i == listeningSocket) {

					t_head identificador;

					int socketCliente = acceptSocket(listeningSocket);
					if (socketCliente == -1) {
						perror("accept");
					}
					identificador = recvHead(socketCliente);
					if (identificador.context == ERROR_HEAD) {
						puts(
								"Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
					} else {
						printf("Conectado a %s.\n",
								identificar(identificador.context));
					}

					if (identificador.context == ESI) { // Si es un ESI, le asigna un id
						idESI++;
						send(socketCliente, &idESI, sizeof(idESI), 0);
						agregarESIAColaDeListos(socketCliente, idESI); //Agrego el nuevo ESI a la cola de listos
					}

					// handle new connections

					FD_SET(socketCliente, &master); // add to master set
					if (socketCliente > fdmax) {	// keep track of the max
						fdmax = socketCliente;
					}
				} else if (i == coordinadorSocket) { // El coordinador me quiere decir algo
					t_head header = recvHead(i);
					if (header.context == ERROR_HEAD) {
						close(i);
						FD_CLR(i, &master); // remove from master set
					} else {
						// we got some data from the coordinador
						switch (header.context) {
						case ACT_GET:
							recv(i, &paqueteGet, header.mSize, 0);
							printf("Se recibió un GET <%s> del ESI %d \n", paqueteGet.clave, paqueteGet.idESI);
							// verificar si la solicitud es valida
							// mandar por si o por no
							break;
						case ACT_SET:
							recv(i, &paqueteSet, header.mSize, 0);
							printf("Se recibió un SET <%s> <%s> del ESI %d\n",paqueteSet.clave, paqueteSet.valor, paqueteSet.idESI);
							break;
						case ACT_STORE:
							recv(i, &paqueteStore, header.mSize, 0);
							printf("Se recibió un STORE <%s> del ESI %d \n", paqueteStore.clave, paqueteStore.idESI);
							break;
						default:
							printf("La solicitud del ESI %d es inválida.\n",
									idESI);
						}
					}
				} else { //Un ESI me quiere decir algo
					t_head header = recvHead(i);
					if (header.context == ERROR_HEAD) {
						close(i);
						FD_CLR(i, &master); // remove from master set
					} else {
						// we got some data from an ESI
						char* clave = malloc(header.mSize);
						switch (header.context) {
						case blockedESI:
							recv(i, clave, header.mSize, 0);
							//(Pendiente) Obtener idESI a partir de su socket
							printf("El ESI %d me informa que queda bloqueado esperando la clave %s.\n", idESI, clave);
							// Sacar ESI de cola de listos, agregar a cola de bloqueados por la clave
							break;
						case okESI:
							printf("El ESI %d finalizo su accion correctamente.\n", idESI);
							break;
						default:
							printf("Error en ESI.\n");
						}
					}
				}
			} // END got new incoming connection
		} // END looping through file descriptors
	} // END for(;;)--and you thought it would never end!

	close(listeningSocket);
	log_destroy(logPlanificador);
//	consola();

	return 0;
}


//************* FUNCIONES *******************

void agregarESIAColaDeListos(int socketESI, int id) {
	t_ESI *nuevoESI = malloc(sizeof(t_ESI));
	nuevoESI->idESI = id;
	nuevoESI->socket = socketESI;
	nuevoESI->esperaClave = NULL;
	nuevoESI->estAntRaf = ESTIMACION_I;
	nuevoESI->estSigRaf = ESTIMACION_I;

	list_add(listos, nuevoESI);
}

//int main(void) {
//
//	int newfd; 			// newly accept()ed socket descriptor
//	struct sockaddr_storage remoteaddr; // client address
//	socklen_t addrlen;
//
//	char buf[256];		// buffer for client data
//	int nbytes;
//
//	char remoteIP[INET6_ADDRSTRLEN];
//
//	int yes = 1;		// for setsockopt() SO_REUSEADDR, below
//	int i, j, rv;
//
//	struct addrinfo hints, *ai, *p;
//
//	// get us a socket and bind it
//	memset(&hints, 0, sizeof hints);
//	hints.ai_family = AF_UNSPEC;
//	hints.ai_socktype = SOCK_STREAM;
//	hints.ai_flags = AI_PASSIVE;
//	if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
//		fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
//		exit(1);
//	}
//
//	for (p = ai; p != NULL; p = p->ai_next) {
//		listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
//		if (listener < 0) {
//			continue;
//		}
//
//		// lose the pesky "address already in use" error message
//		setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
//
//		if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
//			close(listener);
//			continue;
//		}
//		break;
//	}
//
//	// if we got here, it means we didn't get bound
//	if (p == NULL) {
//		fprintf(stderr, "selectserver: failed to bind\n");
//		exit(2);
//	}
//
//	freeaddrinfo(ai); // all done with this
//
//	// listen
//	if (listen(listener, 10) == -1) {
//		perror("listen");
//		exit(3);
//	}
//
//	return 0;
//}
