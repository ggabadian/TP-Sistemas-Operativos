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

	instanciasRegistradas = list_create();
	clavesRegistradas = dictionary_create();

	int listeningSocket = listenSocket(PUERTO);

	listen(listeningSocket, 32);

	puts("Esperando cliente.");

	while (1) {
		t_head identificador;

		int socketCliente = acceptSocket(listeningSocket);
		//sleep(1);
		identificador = recvHead(socketCliente);
		if (identificador.context == ERROR_HEAD) {
			log_error(logCoordinador, "(HANDSHAKE) No se pudo identificar a la entidad. Conexión desconocida.\n");
		} else {
			log_info(logCoordinador, "Conectado a %s.", identificar(identificador.context));
		}

		if (identificador.context == PLANIFICADOR){
			socketPlanificador = socketCliente;
		}

		if (identificador.context == CONSOLA){
			socketConsola = socketCliente;
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
	int statusConsola = 1;

	int* sockfd =malloc(sizeof(int));
	*sockfd = socket;

	switch(id){
	case PLANIFICADOR:
		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador, sockfd);
		if (statusPlanificador) {
			log_error(logCoordinador, "No se pudo crear el thread para Planificador");
		}
		break;
	case ESI:
		statusESI = pthread_create(&thread, NULL, &threadESI, sockfd);

		if (statusESI) {
			log_error(logCoordinador, "No se pudo crear el thread para ESI");
		}
		break;
	case INSTANCIA:
		statusInstancia = pthread_create(&thread, NULL, &threadInstancia, sockfd);
		if (statusInstancia) {
			log_error(logCoordinador, "No se pudo crear el thread para Instancia");
		}

		break;
	case CONSOLA:
		statusConsola = pthread_create(&thread, NULL, &threadConsola, sockfd);
		if (statusConsola) {
			log_error(logCoordinador, "No se pudo crear el thread para la Consola");
		}
		break;
	default:
		log_error(logCoordinador, "No se pudo crear thread: La conexión es desconocida");
	}
}

void* threadPlanificador(void* socket) {
	//int socketPlanificador = *(int*)socket;

	//close(socketPlanificador);

	return NULL;
}

void* threadConsola(void* socket) {
	int socketConsola = *(int*)socket;
	int connected = 1;

	while (connected) {
		t_head header = recvHead(socketConsola);

		switch (header.context){
			case statusClave:
				log_trace(logCoordinador, "Se recibió el pedido 'status clave' desde la consola");
				header.context = okRecibido;
				header.mSize = 0;
				sendHead(socketConsola, header);
				break;

			case cerrarConexion:
				log_trace(logCoordinador, "Se recibió la orden de cerrar la conexión");
				connected = 0;
				break;

			default:
				break;
		}
	}


	close(socketConsola);
	log_trace(logCoordinador, "Se cerró la conexión con la consola");

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
		usleep(100000);// hardcodeado, despues cambiar por sleep(RETARDO);

		switch(header.context){
			case OPERACION_GET:
				recv(socketESI, &paqueteGet, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) GET <%s>", idESI, paqueteGet.clave);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteGet, sizeof(paqueteGet), 0);

				// Recibe respuesta del planificador
				header=recvHead(socketPlanificador);
				puts("Recibi algo del planif");
				switch (header.context){
					case okESI:
						sendOkESI(socketESI);
						puts("ok");
						break;
					case blockedESI:
						sendBlockedESI(socketESI);
						puts("block");
						break;
					default:
						puts("def");
						break;
				}
				break;
			case OPERACION_SET:
				recvSet(socketESI, &paqueteSet);
				log_trace(logDeOperaciones, "(ESI %d) SET <%s> <%s>", idESI, paqueteSet.clave, paqueteSet.valor);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				sendSet(socketPlanificador, &paqueteSet);

				// Recibe respuesta del planificador
				header=recvHead(socketPlanificador);
				switch (header.context){
					case okESI:
						if (distribuirSet(paqueteSet)){
							sendOkESI(socketESI);
						} else {
							sendAbortESI(socketESI);
						}
						break;
					case abortESI:
						sendAbortESI(socketESI);
						break;
					default:
						break;
				}
				break;
			case OPERACION_STORE:
				recv(socketESI, &paqueteStore, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) STORE <%s>", idESI, paqueteStore.clave);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteStore, sizeof(paqueteStore), 0);

				// Recibe respuesta del planificador
				header=recvHead(socketPlanificador);
				switch (header.context){
					case okESI:
						if (distribuirStore(paqueteStore)){
							sendOkESI(socketESI);
						} else {
							sendAbortESI(socketESI);
						}
						break;
					case abortESI:
						sendAbortESI(socketESI);
						break;
					default:
						break;
				}
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
		// inicio mutexInstanciasRegistradas
		registrarInstancia(socketInstancia, nombreDeInstancia);
		// fin mutexInstanciasRegistradas
	} else {
		log_error(logCoordinador, "(INSTANCIA) No se recibió el nombre");
		return NULL;
	}

	header = recvHead(socketInstancia);
	while(header.context != ERROR_HEAD){ // Se queda bloqueado hasta que se desconecte
		switch (header.context){
			case ORDEN_COMPACTAR:
				enviarOrdenCompactar(); // Envia la orden de compactar a todas las instancias
				break;
//			case NRO_ENTRADAS:
//				recv() entradasInstancia;
//				instancia = instanciaConSocket(socket);
//				instancia->entradasLibres = entradasInstancia;
//				break;
			case FIN_COMPACTAR:
				//inicio mutex
				instanciasCompactando --;
				//fin mutex
				break;
			default:
				break;
		}
		header = recvHead(socketInstancia);
	}
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
		//nuevaInstancia->claves = list_create();

		list_add(instanciasRegistradas,nuevaInstancia);
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

	while(index < list_size(instanciasRegistradas)){
		instancia = list_get(instanciasRegistradas, index++);
		if (!strcmp(nombre, instancia->nombre))
			return instancia;
	}

	return NULL;
}

bool distribuirSet(t_set paquete){
	t_instancia *instancia;

	instancia = instanciaConClave(paquete.clave);
	if(instancia != NULL){
		if(!desconectado(instancia->socket)){
			enviarSet(instancia, paquete);
			return true;
		} else { // La clave se encuentra en una instancia desconectada
			dictionary_remove(clavesRegistradas, paquete.clave);
			return false;
		}
	}

	if (!strcmp(ALGORITMO, "EL")) {
		// inicio mutexInstanciasRegistradas
		instancia = equitativeLoad();
		// fin mutexInstanciasRegistradas
	}
	else if (!strcmp(ALGORITMO, "LSU")) {
		// inicio mutexInstanciasRegistradas
		instancia = leastSpaceUsed();
		// fin mutexInstanciasRegistradas
	}
	else if (!strcmp(ALGORITMO, "KE")) {
		// inicio mutexInstanciasRegistradas
		instancia = keyExplicit(paquete.clave);
		// fin mutexInstanciasRegistradas
	}
	else {
		log_error(logCoordinador, "(SET) No se pudo determinar el algoritmo de distribución");
		return false;
	}

	if(instancia != NULL){
		//Para testear
		printf("(Testing) Socket de la instancia elegida: %d\n", instancia->socket);
		printf("(Testing) Cantidad libre de la instancia elegida: %d\n", instancia->entradasLibres);
		enviarSet(instancia, paquete);
		return true;
	} else {
		log_error(logCoordinador, "(SET) No hay ninguna instancia para recibir la solicitud.");
		free(paquete.valor);
		return false;
	}
}

t_instancia* equitativeLoad(){
	if (!list_is_empty(instanciasRegistradas)){
		t_instancia *instancia;
		int auxIndex = indexEquitativeLoad;

		do {
			instancia= list_get(instanciasRegistradas, indexEquitativeLoad ++);
			// Si la instancia que eligió era la ultima, vuelve al principio
			if(!(indexEquitativeLoad < list_size(instanciasRegistradas)))
				indexEquitativeLoad = 0;
		} while(desconectado(instancia->socket) && !(auxIndex == indexEquitativeLoad));

		// Si esto pasa significa que ninguna instancia de la lista está conectada
		if(desconectado(instancia->socket)){
			return NULL;
		} else { // Significa que encontro una instancia conectada
			return instancia;
		}
	} else {
		return NULL;
	}
}

t_instancia* leastSpaceUsed(){
// Como recorre la lista en orden, el desempate por Equitative Load está implícito
	if (!list_is_empty(instanciasRegistradas)){
		t_instancia *instancia;
		t_instancia *instanciaElegida;
		int index = 0;

		do { // (Pendiente) BUG - Rompe si todas las de la lista estan desconectadas
		instanciaElegida = list_get(instanciasRegistradas, index++);
		} while(desconectado(instanciaElegida->socket));

		while(index < list_size(instanciasRegistradas)){
			// Guarda la instancia de ese index y lo incrementa
			instancia = list_get(instanciasRegistradas, index++);

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
	t_list *instanciasConectadas = instanciasActivas();
	if (!list_is_empty(instanciasRegistradas)){
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
	} else {
		return NULL;
	}
}

void enviarSet(t_instancia *instancia, t_set paquete){
	t_head header;
	header.context = OPERACION_SET;
	header.mSize = sizeof(paquete);

	sendHead(instancia->socket, header);
	sendSet(instancia->socket, &paquete);

	instancia->entradasLibres--; //(Pendiente) Sacar

	if (!(claveRegistrada(paquete.clave, instancia)))
		dictionary_put(clavesRegistradas, paquete.clave, instancia);

	free(paquete.valor);
}

bool distribuirStore(t_store paquete){
	t_instancia *instancia;
	t_head header;
	instancia = instanciaConClave(paquete.clave);

	header.context = OPERACION_STORE;
	header.mSize = sizeof(t_store);

	if(instancia != NULL){
		sendHead(instancia->socket, header);
		send(instancia->socket, &paquete, sizeof(paquete), 0);
		return true;
	} else {
		puts("Error: No se encontró la instancia con esa clave.");
		return false;
	}
}

void enviarStore(t_instancia *instancia, t_head header, t_store paquete){
	sendHead(instancia->socket, header);
	send(instancia->socket, &paquete, header.mSize, 0);
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

void sendAbortESI(int socketESI){
	t_head header;

	header.context = abortESI;
	header.mSize = 0;

	sendHead(socketESI, header);
}

bool desconectado (int socket){
	return (send(socket, 0, 0, 0) == -1);
}

void enviarOrdenCompactar(){
	t_instancia *instancia;
	int index = 0;
	t_head header;

	header.context = ORDEN_COMPACTAR;
	header.mSize = 0;

	//informa al Planificador que se inicia la compactación
	sendHead(socketPlanificador, header);

	while(index < list_size(instanciasRegistradas)){
		instancia = list_get(instanciasRegistradas, index++);
		if(!desconectado(instancia->socket)){
			instanciasCompactando++;
			sendHead(instancia->socket, header);
		}
	}

	while(instanciasCompactando > 0) sleep(1); // Se bloquea hasta que terminen de compactar

	header.context = FIN_COMPACTAR;
	header.mSize = 0;

	//informa al Planificador que se terminó la compactación
	sendHead(socketPlanificador, header);
}

t_list *instanciasActivas(){ // Retorna una lista con las instancias actualmente conectadas
	if (!list_is_empty(instanciasRegistradas)){
		t_instancia *instancia;
		int index = 0;
		t_list *instanciasConectadas = list_create();

		while(index < list_size(instanciasRegistradas)){
			instancia = list_get(instanciasRegistradas, index++);
			if(!desconectado(instancia->socket))
				list_add(instanciasConectadas, instancia);
		}

		return instanciasConectadas;
	} else {
		return NULL;
	}
}

t_instancia* instanciaConClave(char *clave){ // Retorna la instancia que contiene esa clave
	if (!list_is_empty(instanciasRegistradas)){
		t_instancia *instancia;
		int index = 0;

		while(index < list_size(instanciasRegistradas)){
			instancia = list_get(instanciasRegistradas, index++);
			if (claveRegistrada(clave, instancia)) {
				return instancia;
			}
		}
		return NULL;
	} else {
		return NULL;
	}
}

bool claveRegistrada(char *clave, t_instancia *instancia){
	return dictionary_get(clavesRegistradas, clave)==instancia;
}
