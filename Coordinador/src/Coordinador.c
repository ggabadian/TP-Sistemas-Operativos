#include "Coordinador.h"

int main(void) {
	//creo el logger
	logCoordinador = log_create("../logs/logCoordinador.log", "Coordinador", true, LOG_LEVEL_TRACE);
	logDeOperaciones = log_create("../logs/logDeOperaciones.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logCoordinador, "Iniciando Coordinador");

	cargarConfig();
	log_info(logCoordinador, "Configuración inicial realizada.");

	if (!strcmp(ALGORITMO, "EL")) {
		log_info(logCoordinador, "(ALGORITMO) Equitative Load.");
	}
	else if (!strcmp(ALGORITMO, "LSU")) {
		log_info(logCoordinador, "(ALGORITMO) Least Space Used.");
	}
	else if (!strcmp(ALGORITMO, "KE")) {
		log_info(logCoordinador, "(ALGORITMO) Key Explicit.");
	}
	else {
		log_error(logCoordinador, "(ALGORITMO) No se pudo cargar.");
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
			log_error(logCoordinador, "(HANDSHAKE) No se pudo identificar a la entidad. Conexión desconocida.\n");
		} else {
			log_info(logCoordinador, "Conectado a %s.", identificar(identificador.context));
		}

		if (identificador.context == PLANIFICADOR){
			socketPlanificador = socketCliente;
		}
		crearThread(identificador.context, socketCliente);
	}

	close(listeningSocket);
	log_destroy(logCoordinador);
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
		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador, &socket);
		if (statusPlanificador) {
			log_error(logCoordinador, "No se pudo crear el thread para Planificador");
		}
		break;
	case ESI:
		statusESI = pthread_create(&thread, NULL, &threadESI, &socket);

		if (statusESI) {
			log_error(logCoordinador, "No se pudo crear el thread para ESI");
		}
		break;
	case INSTANCIA:
		statusInstancia = pthread_create(&thread, NULL, &threadInstancia, &socket);
		if (statusInstancia) {
			log_error(logCoordinador, "No se pudo crear el thread para Instancia");
		}

		break;
	default:
		log_error(logCoordinador, "No se pudo crear thread: La conexión es desconocida");
	}
}

void* threadPlanificador(void* socket) {
	int socketPlanificador = *(int*)socket;

	while (1) {
//		int headPlanificador = recibirHead(ePlanificador->socketPlanificador);
//		hacerAlgo(headPlanificador);
	}

	close(socketPlanificador);

	return NULL;
}

void* threadESI(void* socket) {
	int socketESI = *(int*)socket;
	int connected = 1;
	int idESI;
	if(recv(socketESI, &idESI, sizeof(int), 0) <= 0){
		log_error(logCoordinador, "(ESI) No se recibió su id.");
		return NULL;
	}

	t_get paqueteGet;
	t_set paqueteSet;
	t_store paqueteStore;

	while (connected) {
		t_head header = recvHead(socketESI);

		switch(header.context){
			case ACT_GET:
				recv(socketESI, &paqueteGet, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) GET <%s>", idESI, paqueteGet.clave);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteGet, sizeof(paqueteGet), 0);

				// Recibe respuesta del planificador
				// recv()
				// Si puede, entonces:
					sendOkESI(socketESI);
				// si no:
					//sendBlockedESI(socketESI);
				break;
			case ACT_SET:
				recv(socketESI, &paqueteSet, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) SET <%s> <%s>", idESI, paqueteSet.clave, paqueteSet.valor);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteSet, sizeof(paqueteSet), 0);

				// Recibe respuesta del planificador
				// recv()
				// Si puede, entonces:
					sendOkESI(socketESI);
					assignSet(paqueteSet);
				// si no:
					//sendBlockedESI(socketESI);
				break;
			case ACT_STORE:
				recv(socketESI, &paqueteStore, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) STORE <%s>", idESI, paqueteStore.clave);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteStore, sizeof(paqueteStore), 0);

				// Recibe respuesta del planificador
				// recv()
				// Si puede, entonces:
					sendOkESI(socketESI);
					//assignStore(header, paqueteStore.clave);
				// si no:
					//sendBlockedESI(socketESI);
				break;
			case ERROR_HEAD:
				log_info(logCoordinador, "(ESI %d) Se perdió la conexión.", idESI);
				connected = 0;
				break;
			default:
				log_error(logDeOperaciones, "(ESI %d) Operación inválida", idESI);
		}
	}

	return NULL;
}

void* threadInstancia(void* socket) {
	int socketInstancia = *(int*)socket;
	t_head header;
	char* nombreDeInstancia;

	sendInitInstancia(socketInstancia);

	header = recvHead(socketInstancia);
	if (header.context == nombreInstancia){
		nombreDeInstancia = malloc(header.mSize + 1);
		recv(socketInstancia, nombreDeInstancia, header.mSize, 0);
		nombreDeInstancia[header.mSize] = '\0';
		registrarInstancia(socketInstancia, nombreDeInstancia);
	} else {
		log_error(logCoordinador, "(INSTANCIA) No se recibió el nombre");
		return NULL;
	}

	recvHead(socketInstancia); // Se queda bloqueado hasta que se desconecte
	log_info(logCoordinador, "(%s) Se perdió la conexión.", nombreDeInstancia);

	close(socketInstancia);
	return NULL;
}

void sendInitInstancia(int socket) {

	t_head header;
	t_initInstancia paquete;

	paquete.cantidadEntradas = CANTIDAD_ENTRADAS;
	paquete.sizeofEntrada = BYTES_ENTRADA;

	// Envia el HEAD para que la instancia sepa lo que va a recibir
	header.context = INIT_INSTANCIA;
	header.mSize = sizeof(paquete);
	sendHead(socket, header);

	//Envia el paquete
	send(socket, &paquete, sizeof(paquete), 0);
}

void registrarInstancia(int socket, char* nombre){
	t_instancia *instancia = instanciaRegistrada(nombre);

	// Si el nombre no estaba registrado
	if (instancia == NULL){
		t_instancia *nuevaInstancia = malloc(sizeof(t_instancia)+ strlen(nombre) - sizeof(char*));
		nuevaInstancia->nombre = nombre;
		nuevaInstancia->socket = socket;
		nuevaInstancia->entradasLibres = CANTIDAD_ENTRADAS;

		list_add(instanciasConectadas,nuevaInstancia);
	} else {
		// Actualiza el socket
		instancia->socket = socket;
		log_info(logCoordinador, "(%s) Se reincorporó.", instancia->nombre);
	}
	//free(nuevaInstancia); // Hay que liberarla pero aca no es
}

t_instancia *instanciaRegistrada(char* nombre){
	t_instancia *instancia;
	int index = 0;

	while(index < list_size(instanciasConectadas)){
		instancia = list_get(instanciasConectadas, index++);
		if (!strcmp(nombre, instancia->nombre))
			return instancia;
	}

	return NULL;
}

void assignSet(t_set paquete){
	t_instancia *instancia;

	if (!strcmp(ALGORITMO, "EL")) {
		instancia = equitativeLoad();
	}
	else if (!strcmp(ALGORITMO, "LSU")) {
		instancia = leastSpaceUsed();
	}
	else if (!strcmp(ALGORITMO, "KE")) {
		instancia = keyExplicit(paquete.clave);
	}
	else {
		log_error(logCoordinador, "(SET) No se pudo determinar el algoritmo de distribución");
		return;
	}

	if(instancia != NULL){
		//Para testear
		printf("(Testing) Socket de la instancia elegida: %d\n", instancia->socket);
		printf("(Testing) Cantidad libre de la instancia elegida: %d\n", instancia->entradasLibres);
		sendSet(instancia, paquete);
	} else {
		log_error(logCoordinador, "(SET) No hay ninguna instancia para recibir la solicitud.");
	}
}

t_instancia* equitativeLoad(){
	if (!list_is_empty(instanciasConectadas)){
		t_instancia *instancia;
		// Si la instancia anterior que eligio era la ultima, vuelve al principio
//		if(!(indexEquitativeLoad < list_size(instanciasConectadas)))
//			indexEquitativeLoad = 0;

		do { // (Pendiente) BUG - Rompe si todas las de la lista estan desconectadas
		if(!(indexEquitativeLoad < list_size(instanciasConectadas)))
			indexEquitativeLoad = 0;
		instancia= list_get(instanciasConectadas, indexEquitativeLoad ++);
		} while(desconectado(instancia->socket));

		return instancia;
	} else {
		return NULL;
	}
}

t_instancia* leastSpaceUsed(){
// Como recorre la lista en orden, el desempate por Equitative Load está implícito
	if (!list_is_empty(instanciasConectadas)){
		t_instancia *instancia;
		t_instancia *instanciaElegida;
		int index = 0;

		do { // (Pendiente) BUG - Rompe si todas las de la lista estan desconectadas
		instanciaElegida = list_get(instanciasConectadas, index++);
		} while(desconectado(instanciaElegida->socket));

		while(index < list_size(instanciasConectadas)){
			// Guarda la instancia de ese index y lo incrementa
			instancia = list_get(instanciasConectadas, index++);

			if (!desconectado(instancia->socket) && (instancia->entradasLibres > instanciaElegida->entradasLibres))
				// Guarda la nueva instancia con mayor entradas libres como candidata
				instanciaElegida = instancia;
		}
		return instanciaElegida;
	} else {
		return NULL;
	}
}

t_instancia* keyExplicit(char* clave){
	int cantidadDeLetras = 26;
	// Cantidad de letras de las instancias que almacenarán claves (excepto la última)
	int letrasPorInstancia = 0;

	double division = cantidadDeLetras/(double)list_size(instanciasConectadas);

	// Distribuye apropiadamente la cantidad de letras segun la cantidad de instancias
	letrasPorInstancia = (int)ceil(division); // Redondea siempre para arriba

	// Guarda la primera letra de la clave
	char* letra = string_substring_until(clave, 1);
	string_to_lower(letra); // La pasa a minúscula
	int nroLetra = *letra - 'a' + 1;

	int indexInstancia = (int)(ceil(nroLetra/(double)letrasPorInstancia)) - 1;

	free(letra);

	return list_get(instanciasConectadas, indexInstancia);
}

void sendSet(t_instancia *instancia, t_set paquete){
	t_head header;
	header.context = ACT_SET;
	header.mSize = sizeof(paquete);

	sendHead(instancia->socket, header);
	send(instancia->socket, &paquete, sizeof(paquete), 0);

	instancia->entradasLibres--; //(Pendiente) Guardar clave
}

void assignStore(t_head header, char* clave){ //(Pendiente)
/*	t_instancia *instancia;
	//(Pendiente) Analizar a que instancia se va a enviar
	//Para testear la asigno con EL
	//instancia = equitativeLoad(instanciasConectadas);
	if(instancia != NULL){
		sendStore(instancia, header, clave);
	} else {
		puts("Error: No hay ninguna instancia para recibir la solicitud.");
	}*/
}

void sendStore(t_instancia *instancia, t_head header, char* clave){
	sendHead(instancia->socket, header);
	send(instancia->socket, clave, header.mSize, 0);
}

void sendBlockedESI(int socketESI){
	t_head header;

	header.context = blockedESI;
	header.mSize = 0;

	sendHead(socketESI, header);
}

void sendOkESI(int socketESI){
	t_head header;

	header.context = okESI;
	header.mSize = 0;

	sendHead(socketESI, header);
}

bool desconectado (int socket){
	return (send(socket, 0, 0, 0) == -1);
}
