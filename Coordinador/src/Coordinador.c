#include "Coordinador.h"

//esta funcion tira un warning que queda para corregir

int main(void) {
	t_log* logCoordinador;//puede que haya que ponerla global si se usa en alguna funcion
	//creo el logger
	logCoordinador = log_create("../log/logDeOperaciones.log", "Coordinador", true, LOG_LEVEL_TRACE);
	//se usa para escribir en el archivo de log y lo muestra por pantalla
	log_trace(logCoordinador, "Iniciando Coordinador");
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
//(Pendiente) BUG - Si se abre una instancia mientras se esta ejecutando un ESI
//					se rompe la conexion ESI-Coordinador

void crearThread(e_context id, int socket) {
	pthread_t thread;
	int statusPlanificador = 1;
	int statusESI = 1;
	int statusInstancia = 1;

	switch(id){
	case PLANIFICADOR:
		statusPlanificador = pthread_create(&thread, NULL, &threadPlanificador, &socket);
		if (statusPlanificador) {
			puts("Error en la creación de thread para Planificador");
			//(Pendiente) log error
		}
		break;
	case ESI:
		statusESI = pthread_create(&thread, NULL, &threadESI, &socket);

		if (statusESI) {
			puts("Error en la creación de thread para ESI");
			//(Pendiente) log error
		}
		break;
	case INSTANCIA:
		statusInstancia = pthread_create(&thread, NULL, &threadInstancia, &socket);
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
		puts("Error al recibir id del ESI");
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
				printf("Se recibió un GET <%s> del ESI %d \n", paqueteGet.clave, idESI);
				// Consultar al planificador
				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteGet, sizeof(paqueteGet), 0);

				// Recibir respuesta del planificador
				// Si puede, entonces:
					sendOkESI(socketESI);
				// si no:
					//sendBlockedESI(socketESI);
				//(Pendiente) log operacion
				break;
			case ACT_SET:
				recv(socketESI, &paqueteSet, header.mSize, 0);
				printf("Se recibió un SET <%s> <%s> del ESI %d\n", paqueteSet.clave, paqueteSet.valor, idESI);

				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteSet, sizeof(paqueteSet), 0);

				sendOkESI(socketESI);

				assignSet(paqueteSet);
				//(Pendiente) log operacion
				break;
			case ACT_STORE:
				recv(socketESI, &paqueteStore, header.mSize, 0);
				printf("Se recibió un STORE <%s> del ESI %d\n", paqueteStore.clave, idESI);

				sendHead(socketPlanificador, header);
				send(socketPlanificador, &paqueteStore, sizeof(paqueteStore), 0);

				//assignStore(header, paqueteStore.clave);

				sendOkESI(socketESI);

				//(Pendiente) log operacion

				break;
			case ERROR_HEAD:
				printf("Se perdió la conexión con el ESI %d.\n", idESI);
				connected = 0;
				break;
			default:
				printf("La solicitud del ESI %d es inválida.\n", idESI);
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
		puts("Error: No se recibió el nombre de la instancia.");
		return NULL;
	}

	recvHead(socketInstancia); // Se queda bloqueado hasta que se desconecte
	printf("Se perdió la conexión con %s.\n", nombreDeInstancia);

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

	puts("Configuración inicial enviada a Instancia.");
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
		printf("Instancia registrada: <%s> ", nuevaInstancia->nombre);
	} else {
		// Actualiza el socket
		instancia->socket = socket;
		printf("La instancia <%s> se reincorporó al sistema ", instancia->nombre);
	}

	printf("con socket %d.\n", socket);
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
		puts("Error: No se pudo determinar el algoritmo de distribución");
		return;
	}

	if(instancia != NULL){
		//Para testear
		printf("(Testing) Socket de la instancia elegida: %d\n", instancia->socket);
		printf("(Testing) Cantidad libre de la instancia elegida: %d\n", instancia->entradasLibres);
		sendSet(instancia, paquete);
	} else {
		puts("Error: No hay ninguna instancia para recibir la solicitud.");
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
