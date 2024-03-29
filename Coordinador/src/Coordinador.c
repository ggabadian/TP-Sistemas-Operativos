#include "Coordinador.h"

int main(int argc, char* argv[]) {
	pthread_mutex_init(&mutexInstanciasConectadas, NULL);

	logCoordinador = log_create("../logs/logCoordinador.log", "Coordinador", true, LOG_LEVEL_TRACE);
	logDeOperaciones = log_create("../logs/logDeOperaciones.log", "Coordinador", true, LOG_LEVEL_TRACE);
	log_trace(logCoordinador, "Iniciando Coordinador");

	cargarConfig(argv[1]);
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
			if(identificador.context != CONSOLA)
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
	return NULL;
}

void* threadConsola(void* socket) {
	int socketConsola = *(int*)socket;
	int connected = 1;

	while (connected) {
		t_head header = recvHead(socketConsola);
		char* clave = malloc(header.mSize);
		switch (header.context){
			case statusClave:
				log_trace(logCoordinador, "Se recibió el pedido 'status clave' desde la consola");
				recv(socketConsola, clave, header.mSize, MSG_WAITALL);
				sendStatus(clave);
				break;

			default:
				connected = 0;
		}
		free(clave);
	}


	close(socketConsola);
	log_trace(logCoordinador, "Se cerró la conexión con el planificador");

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
		usleep(RETARDO*1000); // El retardo es en milisegundos

		switch(header.context){
			case OPERACION_GET:
				recv(socketESI, &paqueteGet, header.mSize, 0);
				log_trace(logDeOperaciones, "(ESI %d) GET <%s>", idESI, paqueteGet.clave);
				// Consulta al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteGet, sizeof(paqueteGet), 0);

				// Recibe respuesta del planificador
				header=recvHead(socketPlanificador);
				switch (header.context){
					case okESI:
						sendOkESI(socketESI);
						break;
					case blockedESI:
						sendBlockedESI(socketESI);
						break;
					default:
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
	t_instancia *instancia;

	sendInitInstancia(socketInstancia);

	header = recvHead(socketInstancia);
	if (header.context == nombreInstancia){
		nombreDeInstancia = malloc(header.mSize + 1);
		recv(socketInstancia, nombreDeInstancia, header.mSize, 0);
		nombreDeInstancia[header.mSize] = '\0';
		pthread_mutex_lock(&mutexInstanciasConectadas);
		registrarInstancia(socketInstancia, nombreDeInstancia);
		pthread_mutex_unlock(&mutexInstanciasConectadas);
	} else {
		log_error(logCoordinador, "(INSTANCIA) No se recibió el nombre");
		return NULL;
	}
	instancia = instanciaConSocket(socketInstancia);
	header = recvHead(socketInstancia);
	while(header.context != ERROR_HEAD){ // Se queda bloqueado hasta que se desconecte
		switch (header.context){
			case ORDEN_COMPACTAR:
				enviarOrdenCompactar(); // Envia la orden de compactar a todas las instancias
				break;
			case NRO_ENTRADAS:
				if (instancia != NULL)
					instancia->entradasLibres = header.mSize;
				break;
			case FIN_COMPACTAR:
				instanciasCompactando --;
				break;
			case statusValor:
				valorDeClave = malloc(header.mSize);
				recv(socketInstancia, valorDeClave, header.mSize, 0);
				break;
			case STORE_OK:
				operacionStore = SUCCESS;
				break;
			case STORE_FAIL:
				operacionStore = ERROR;
				break;
			default:
				break;
		}
		header = recvHead(socketInstancia);
	}
	log_info(logCoordinador, "(%s) Se perdió la conexión.", nombreDeInstancia);

	close(socketInstancia);
	instancia->socket = 0;

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
	t_head header;
	header.context = REINCORPORACION_INSTANCIA;
	header.mSize = 0;
	int nroClaves = 0;
	int memClaves = 0;
	char* clavesParaMandar;

	// Si el nombre no estaba registrado
	if (instancia == NULL){
		t_instancia *nuevaInstancia = malloc(sizeof(t_instancia)+ strlen(nombre) - sizeof(char*));
		nuevaInstancia->nombre = nombre;
		nuevaInstancia->socket = socket;
		nuevaInstancia->entradasLibres = CANTIDAD_ENTRADAS;

		list_add(instanciasRegistradas,nuevaInstancia);
	} else {
		// Actualiza el socket
		instancia->socket = socket;
		free(instancia->nombre);
		instancia->nombre = nombre;
		log_info(logCoordinador, "(%s) Se reincorporó.", instancia->nombre);
		nroClaves = nroClavesAsociadas(instancia);
		memClaves = memClavesAsociadas(instancia);
		if(nroClaves > 0){
			clavesParaMandar = clavesAsociadas(instancia);
			header.mSize = nroClaves+memClaves+1;
		}
	}
	sendHead(socket, header);
	if (header.mSize != 0) {
		send(socket, clavesParaMandar, header.mSize, 0);
		free(clavesParaMandar);
	}

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
			//puts("instanciaConClave");
			enviarSet(instancia, paquete);
			log_info(logCoordinador, "(SET) Instancia elegida: %s (Entradas libres: %d).", instancia->nombre, instancia->entradasLibres);
			return true;
		} else { // La clave se encuentra en una instancia desconectada
			dictionary_remove(clavesRegistradas, paquete.clave);
			return false;
		}
	}

	if (!strcmp(ALGORITMO, "EL")) {
		pthread_mutex_lock(&mutexInstanciasConectadas);
		instancia = equitativeLoad();
		pthread_mutex_unlock(&mutexInstanciasConectadas);
	}
	else if (!strcmp(ALGORITMO, "LSU")) {
		pthread_mutex_lock(&mutexInstanciasConectadas);
		instancia = leastSpaceUsed();
		pthread_mutex_unlock(&mutexInstanciasConectadas);
	}
	else if (!strcmp(ALGORITMO, "KE")) {
		pthread_mutex_lock(&mutexInstanciasConectadas);
		instancia = keyExplicit(paquete.clave);
		pthread_mutex_unlock(&mutexInstanciasConectadas);
	}
	else {
		log_error(logCoordinador, "(SET) No se pudo determinar el algoritmo de distribución");
		return false;
	}

	if(instancia != NULL){
		enviarSet(instancia, paquete);
		log_info(logCoordinador, "(SET) Instancia elegida: %s (Entradas libres: %d).", instancia->nombre, instancia->entradasLibres);
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
		int auxIndex = 0;

		do {
		instanciaElegida = list_get(instanciasRegistradas, index++);
		if(!(index < list_size(instanciasRegistradas)))
						index = 0;

		} while(desconectado(instanciaElegida->socket) && (auxIndex != index));

		// Si esto pasa significa que ninguna instancia de la lista está conectada
		if(desconectado(instanciaElegida->socket)){
			return NULL;
		} else { // Significa que encontro una instancia conectada
			while(index < list_size(instanciasRegistradas)){
				// Guarda la instancia de ese index y lo incrementa
				instancia = list_get(instanciasRegistradas, index++);

				if (!desconectado(instancia->socket) && (instancia->entradasLibres > instanciaElegida->entradasLibres))
					// Guarda la nueva instancia con mayor entradas libres como candidata
					instanciaElegida = instancia;
			}
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
		operacionStore = WAITING;
		sendHead(instancia->socket, header);
		send(instancia->socket, &paquete, sizeof(paquete), 0);
		while(operacionStore == WAITING) usleep(100000); // Espera su respuesta
		if (operacionStore == SUCCESS){
			return true;
		} else {
			return false;
		}
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

	while(instanciasCompactando > 0) usleep(100000); // Se bloquea hasta que terminen de compactar

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

t_instancia* instanciaConSocket(int socket){ // Retorna la instancia con ese socket
	if (!list_is_empty(instanciasRegistradas)){
		t_instancia *instancia;
		int index = 0;

		while(index < list_size(instanciasRegistradas)){
			instancia = list_get(instanciasRegistradas, index++);
			if((!desconectado(instancia->socket)) & (instancia->socket == socket))
				return instancia;
		}
		return NULL;
	} else {
		return NULL;
	}
}

void sendStatus(char* clave){
	t_instancia *instanciaActual = instanciaConClave(clave);
	t_instancia *instanciaPosible;
	int sizeValor = 0;
	int sizeNombreInstancia;
	t_head header;

	if (instanciaActual != NULL) {
		instanciaPosible = instanciaActual;
		if (!desconectado(instanciaActual->socket)){
			header.context = statusValor;
			header.mSize = MAX_CLAVE;
			sendHead(instanciaActual->socket, header);
			send(instanciaActual->socket, clave, header.mSize, 0);
			while(valorDeClave == NULL) usleep(100000);
			sizeValor = strlen(valorDeClave) + 1;
		}
	} else {
		instanciaPosible = distribuirStatus(clave);
	}

	int packageSize = sizeof(sizeValor) + sizeValor + 2 * (sizeof(sizeNombreInstancia));
	if (instanciaActual != NULL) packageSize += strlen(instanciaActual->nombre) + 1;
	if (instanciaPosible != NULL) packageSize += strlen(instanciaPosible->nombre) + 1;

	char *serializedPackage = malloc(packageSize);
	int offset = 0;
	int sizeToSend;

	// Valor
	sizeToSend = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(sizeValor), sizeToSend);
	offset += sizeToSend;

	if (sizeValor != 0){
		sizeToSend = sizeValor;
		memcpy(serializedPackage + offset, valorDeClave, sizeToSend);
		offset += sizeToSend;
	}

	// Nombre instancia actual
	if (instanciaActual != NULL){
		sizeNombreInstancia = strlen(instanciaActual->nombre) + 1;
	} else {
		sizeNombreInstancia = 0;
	}
	sizeToSend = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(sizeNombreInstancia), sizeToSend);
	offset += sizeToSend;

	if (sizeNombreInstancia != 0){
		sizeToSend = sizeNombreInstancia;
		memcpy(serializedPackage + offset, instanciaActual->nombre, sizeToSend);
		offset += sizeToSend;
	}

	// Nombre posible instancia
	if (instanciaPosible != NULL){
		sizeNombreInstancia = strlen(instanciaPosible->nombre) + 1;
	} else {
		sizeNombreInstancia = 0;
	}
	sizeToSend = sizeof(uint32_t);
	memcpy(serializedPackage + offset, &(sizeNombreInstancia), sizeToSend);
	offset += sizeToSend;

	if (sizeNombreInstancia != 0){
		sizeToSend = sizeNombreInstancia;
		memcpy(serializedPackage + offset, instanciaPosible->nombre, sizeToSend);
		offset += sizeToSend;
	}

	send(socketConsola, serializedPackage, packageSize, 0);

	free(valorDeClave);
	valorDeClave = NULL;
	free(serializedPackage);

}

t_instancia* distribuirStatus(char* clave){
	t_instancia *instancia;
	int auxIndexEquitativeLoad = 0;

		if (!strcmp(ALGORITMO, "EL")) {
			pthread_mutex_lock(&mutexInstanciasConectadas);
			auxIndexEquitativeLoad = indexEquitativeLoad;
			instancia = equitativeLoad();
			indexEquitativeLoad = auxIndexEquitativeLoad; // Para que no tenga efecto real
			pthread_mutex_unlock(&mutexInstanciasConectadas);
		}
		else if (!strcmp(ALGORITMO, "LSU")) {
			pthread_mutex_lock(&mutexInstanciasConectadas);
			instancia = leastSpaceUsed();
			pthread_mutex_unlock(&mutexInstanciasConectadas);
		}
		else if (!strcmp(ALGORITMO, "KE")) {
			pthread_mutex_lock(&mutexInstanciasConectadas);
			instancia = keyExplicit(clave);
			pthread_mutex_unlock(&mutexInstanciasConectadas);
		}
		else {
			log_error(logCoordinador, "(STATUS) No se pudo determinar el algoritmo de distribución");
			return NULL;
		}

		return instancia;
}

char* clavesAsociadas(t_instancia* instancia){
	int cantidadDeClaves = nroClavesAsociadas(instancia);
	char* clavesConcatenadas;
	int index = 0;
	int size = clavesRegistradas->table_max_size;
	int memoriaNecesaria = memClavesAsociadas(instancia);
	if (cantidadDeClaves > 0){
		clavesConcatenadas = malloc(memoriaNecesaria+cantidadDeClaves+1);
		clavesConcatenadas[0]='\0';
	}

	while(index < size){
		t_hash_element *element = clavesRegistradas->elements[index];
		if (cantidadDeClaves > 0){
			while(element != NULL){
				t_hash_element *nextElement = element->next;
				if(instancia == (t_instancia*)(element->data)){
					char* coma = malloc(2);
						strcpy(coma,",");
					strcat(clavesConcatenadas, element->key);
					strcat(clavesConcatenadas, coma);
					free(coma);
				}
				element = nextElement;
			}
		}
		index++;
	}
	if (cantidadDeClaves > 0){
		return clavesConcatenadas;
	} else {
		return NULL;
	}
}

int nroClavesAsociadas(t_instancia* instancia){
	int cantidadDeClaves = 0;
	int index = 0;
	int size = clavesRegistradas->table_max_size;
	while(index < size){
		t_hash_element *element = clavesRegistradas->elements[index];
		while(element != NULL){
			t_hash_element *nextElement = element->next;
			if(instancia == (t_instancia*)(element->data)){
				cantidadDeClaves ++;
			}
			element = nextElement;
		}
		index++;
	}
	return cantidadDeClaves;
}
int memClavesAsociadas(t_instancia* instancia){
	int memoriaClaves = 0;
	int index = 0;
	int size = clavesRegistradas->table_max_size;
	while(index < size){
		t_hash_element *element = clavesRegistradas->elements[index];
		while(element != NULL){
			t_hash_element *nextElement = element->next;
			if(instancia == (t_instancia*)(element->data)){
				memoriaClaves += strlen(element->key);
			}
			element = nextElement;
		}
		index++;
	}
	return memoriaClaves;
}
