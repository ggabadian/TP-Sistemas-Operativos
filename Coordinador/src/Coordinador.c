#include "Coordinador.h"

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Las instancias conectadas se guardan en esta lista
t_list *instanciasConectadas;


int main(void) {
	puts("Iniciando coordinador...");
	cargarConfig();
	puts("Configuración inicial realizada.");

	if (!strcmp(ALGORITMO, "EL")) {
		puts("Se utilizará el algoritmo de distribución Equitative Load.");
	}
	else if (!strcmp(ALGORITMO, "LSU")) {
		puts("Se utilizará el algoritmo de distribución Least Space Used.");
	}
	else if (!strcmp(ALGORITMO, "KE")) {
		puts("Se utilizará el algoritmo de distribución Key Explicit.");
	}
	else {
		puts("Error: No se pudo determinar el algoritmo de distribución.");
		return ERROR;
	}

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
//(Pendiente) BUG - Si se abre una instancia mientras se esta ejecutando un ESI
//					se rompe la conexion ESI-Coordinador

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
				// Consultar al planificador
				// Recibir respuesta del planificador
				// Si puede, entonces:
					asignarSolicitud();
				// si no:
					//informar bloqueo
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
//	int connected = 1;

	sendInitInstancia(*socketInstancia);

	registrarInstancia(*socketInstancia);

//	while (connected) {
//		int headInstancia = recibirHead(eInstancia->socketInstancia);
//		hacerAlgo(headInstancia);

		//connected = 0;
//	}

//	close(*socketInstancia);
	return NULL;
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

	//free(nuevaInstancia); // Hay que liberarla pero aca no es

}

void asignarSolicitud(){
		if (!strcmp(ALGORITMO, "EL")) {
			equitativeLoad();
		}
		else if (!strcmp(ALGORITMO, "LSU")) {
			leastSpaceUsed();
		}
		else if (!strcmp(ALGORITMO, "KE")) {
			keyExplicit();
		}
		else {
			puts("Error: No se pudo determinar el algoritmo de distribución");
			//(Pendiente) log error
		}
}

void equitativeLoad(){
	if (!list_is_empty(instanciasConectadas)){
		t_instancia *instanciaElegida;

		// Agarra el primer elemento de la lista (el mas "viejo")
		instanciaElegida = list_remove(instanciasConectadas, 0);

		// Lo devuelve a la lista en el ultimo lugar (el mas "reciente")
		list_add(instanciasConectadas, instanciaElegida);

		// Esto es para ver que funciona
		printf("El socket de la instancia elegida es: %d\n", instanciaElegida->socket);

		// Envia la solicitud a dicho elemento
		//enviarSolicitud(instanciaElegida, ...);

	} else {
		puts("Error: No hay ninguna instancia para recibir la solicitud.");
	}

}

void leastSpaceUsed(){
	puts("LSU");
}

void keyExplicit(){
	puts("KE");
}
