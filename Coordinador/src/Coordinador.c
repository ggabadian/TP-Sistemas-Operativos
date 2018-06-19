#include "Coordinador.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Las instancias conectadas se guardan en esta lista
t_list *instanciasConectadas;


int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();

	instanciasConectadas = list_create();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 32);

	puts("Esperando cliente.");

	while (1) {
		t_head identificador;

		int socketCliente = acceptSocket(listeningSocket);

		identificador = recvHead(socketCliente);
		if (identificador.context == ERROR_HEAD) {
			puts("Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
			//(Pendiente) log error
		} else {
			printf("Conectado a %s.\n", identificar(identificador.context));
		}

		crearThread(identificador.context, socketCliente);
	}

	close(listeningSocket);
	return 0;
}

//(Pendiente) BUG - Valgrind dice que podria estar perdiendo memoria
//(Pendiente) Semaforos para el manejo de la lista de instancias conectadas

void crearThread(e_context id, int socket) {
	pthread_t thread;
	int statusPlanificador = 1;
	int statusESI = 1;
	int statusInstancia = 1;

	switch(id){
	case PLANIFICADOR:
		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador,
				(void*) &socket);
		if (statusPlanificador) {
			puts("Error en la creación de thread para Planificador");
			//(Pendiente) log error
		}
		break;
	case ESI:
		statusESI = pthread_create(&thread, NULL, &threadESI, (void*) &socket);

		if (statusESI) {
			puts("Error en la creación de thread para ESI");
			//(Pendiente) log error
		}
		break;
	case INSTANCIA:
		statusInstancia = pthread_create(&thread, NULL, &threadInstancia,
				(void*) &socket);
		if (statusInstancia) {
			puts("Error en la creación de thread para Instancia");
			//(Pendiente) log error
		}

		break;
	default:
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
	int connected = 1;
	int idESI;
	recv(*socketESI, &idESI, sizeof(int), 0);

	while (connected) {
		t_head header = recvHead(*socketESI);
		char *dato = malloc(header.mSize);
		t_set paqueteSet;

		switch(header.context){
			case ACT_GET:
				recv(*socketESI, dato, header.mSize, 0);
				printf("Se recibió un GET <%s> del ESI %d \n", dato, idESI);
				//(Pendiente) log operacion
				break;
			case ACT_SET:
				recv(*socketESI, &paqueteSet, header.mSize, 0);
				printf("Se recibió un SET <%s> <%s> del ESI %d\n", paqueteSet.clave, paqueteSet.valor, idESI);
				//(Pendiente) log operacion
				break;
			case ACT_STORE:
				recv(*socketESI, dato, header.mSize, 0);
				printf("Se recibió un STORE <%s> del ESI %d\n", dato, idESI);
				//(Pendiente) log operacion
				break;
			default:
				printf("Se perdió la conexión con el ESI %d.\n", idESI);
				connected = 0;
		}
		free(dato);
	}

	return NULL;
}

void* threadInstancia(void* socket) {
	int* socketInstancia = (int*) socket;
	int connected = 1;

	sendInitInstancia(*socketInstancia);

	registrarInstancia(*socketInstancia);

	while (connected) {
//		int headInstancia = recibirHead(eInstancia->socketInstancia);
//		hacerAlgo(headInstancia);

		//connected = 0;
	}

	close(*socketInstancia);
	free(socketInstancia);
	return NULL;
}

void recibirMensaje(int socket) {
	char *package = malloc(1024);
	while (1) {
		recv(socket, (void*) package, 4, 0);
		printf("%s", package);
	}
	close(socket);
	free(package);
}

void sendInitInstancia(int socket) {

	t_head header;
	t_initInstancia paquete;

	paquete.cantidadEntradas = CANTIDAD_ENTRADAS;
	paquete.sizeofEntrada = BYTES_ENTRADA;

	// Envia el HEAD para que la instancia sepa lo que va a recibir
	header.context = initDatosInstancia;
	header.mSize = sizeof(paquete);
	sendHead(socket, header);

	//Envia el paquete
	send(socket, &paquete, sizeof(paquete), 0);

	puts("Configuración inicial enviada a Instancia.");
}

void registrarInstancia(int socket){
	t_instancia *nuevaInstancia = malloc(sizeof(t_instancia));
	nuevaInstancia->socket = socket;
	nuevaInstancia->entradasLibres = CANTIDAD_ENTRADAS;

	//(Pendiente) Semaforo
	list_add(instanciasConectadas,nuevaInstancia);

	free(nuevaInstancia);

}
