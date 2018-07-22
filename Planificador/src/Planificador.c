#include "Planificador.h"

// Defino los nombres de las listas a utilizar
t_list *listos;
t_dictionary *colasBloqueados;
t_list *finalizados;
t_dictionary *clavesBloqueadas;

bool planificacionPausada;
int clock; //clock del sistema. Se incrementa cada vez que se envia una orden de ejecucion. (cuenta la cantidad total de ordenes enviadas
					 //tambien sirve para medir el tiempo que un esi lleva esperando en la cola de listos. (debe guardarse en su t_esi y calcular la diferencia)
t_ESI *proximoESI;
t_ESI *running;

int main() {

	bool hayQuePlanificar=false;
	bool hayQueEnviarOrdenDeEjecucion=false;
	bool conDesalojo;
	clock=0;
	proximoESI=NULL;
	running=NULL;

	//creo el logger//puede que haya que ponerla global si se usa en alguna funcion
	t_log* logPlanificador;

	//se usa para escribir en el archivo de log y lo muestra por pantalla
	logPlanificador = log_create("../logs/logDePlanificador.log","Planificador", true, LOG_LEVEL_TRACE);

	log_trace(logPlanificador, "Iniciando Planificador");

	// Cargo la configuración del Planificador desde el archivo config
	cargarConfigPlanificador();

	if (strcmp(ALGORITMO, "SJF-CD") == 0) {
		conDesalojo = true;
	}
	else{
		conDesalojo = false;
	}

	// Creo las diferentes listas a ser utilizadas
	listos = list_create();
	colasBloqueados = dictionary_create(); //se entra por clave y devuelve cola de esis bloqueados en espera de la clave
	finalizados = list_create();


	clavesBloqueadas = dictionary_create(); //se entra por clave y devuelve el id del esi que la tiene

	t_head headerAEnviar;
	headerAEnviar.context = PLANIFICADOR;
	headerAEnviar.mSize = 0;

	log_info(logPlanificador, "PUERTO= %s\n", PUERTO);
	log_info(logPlanificador, "ALGORITMO= %s\n", ALGORITMO);
	log_info(logPlanificador, "ALFA= %d\n", ALFA);
	log_info(logPlanificador, "ESTIMACION= %d\n", ESTIMACION_I);
	log_info(logPlanificador, "IP COORDINADOR= %s\n", IP_COORDINADOR);
	log_info(logPlanificador, "PUERTO COORDINADOR= %s\n", PUERTO_COORDINADOR);

	//Agrego las claves bloqueadas por archivo configuración a la lista de claves
	//En la descripción agrego que son por archivo CONFIG ya que después servirá para calular el Deadlock
	for (int claveI = 0; CL_BLOQUEADAS[claveI] != NULL; claveI++) {
		log_info(logPlanificador, "CLAVES BLOQUEADAS= %s", CL_BLOQUEADAS[claveI]);
		dictionary_put(clavesBloqueadas, CL_BLOQUEADAS[claveI], "POR ARCHIVO CONFIG");
	}

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR); //Envío solicitud de conexión al Coordinador
	printf("Conectado a Coordinador. \n");
	sendHead(coordinadorSocket, headerAEnviar); // Le aviso al Coordinador que soy el Planificador

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
	fdmax = (listeningSocket > coordinadorSocket ?listeningSocket : coordinadorSocket);

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
						if(list_is_empty(listos) && running==NULL){
							hayQueEnviarOrdenDeEjecucion=true; //cuando se conecta el primer esi, si o si hay que planificar y enviar orden
							hayQuePlanificar=true;
						}
						agregarNuevoESIAColaDeListos(socketCliente, idESI); //Agrego el nuevo ESI a la cola de listos
						if (conDesalojo){
							hayQuePlanificar=true;
						}
					}

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
						case OPERACION_GET:
							recv(i, &paqueteGet, header.mSize, 0);
							printf("Se recibió un GET <%s> del ESI %d \n",paqueteGet.clave, paqueteGet.idESI);

							// verificar si la solicitud es valida
							if (dictionary_get(clavesBloqueadas,paqueteGet.clave) == NULL) { //nadie tiene tomada la clave
								dictionary_put(clavesBloqueadas,paqueteGet.clave, &paqueteGet.idESI);
								header.context=okESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);

							} else if (*(int*) (dictionary_get(clavesBloqueadas,paqueteGet.clave)) == paqueteGet.idESI) { //el esi que pide es el que tiene tomada la clave (la esta pidiendo por segunda vez
								header.context=okESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);

							} else { // otro esi tiene bloqueada la clave
								header.context=blockedESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);
								bloquearESI(paqueteGet.clave); //esto solo agrega a la cola de bloqueados
							}

							break;

						case OPERACION_SET:
							recvSet(i, &paqueteSet);
							printf("Se recibió un SET <%s> <%s> del ESI %d\n",
									paqueteSet.clave,
									paqueteSet.valor,
									paqueteSet.idESI);
							// verificar si la solicitud es valida
							if (*(int*) (dictionary_get(clavesBloqueadas,paqueteSet.clave)) == paqueteSet.idESI) { //el esi que pide es el que tiene tomada la clave
								header.context=okESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);
							} else { //otro esi tiene bloqueada la clave o no la tiene nadie
								header.context=abortESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);
							}

							//Usar donde corresponda: free(paqueteSet.valor);
							break;
						case OPERACION_STORE:
							recv(i, &paqueteStore, header.mSize, 0);
							printf("Se recibió un STORE <%s> del ESI %d \n",
									paqueteStore.clave, paqueteStore.idESI);

							// verificar si la solicitud es valida
							if (*(int*) (dictionary_get(clavesBloqueadas,paqueteSet.clave)) == paqueteSet.idESI) { //el esi que pide es el que tiene tomada la clave
								dictionary_remove(clavesBloqueadas,paqueteSet.clave);
								header.context=okESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);
								//cuando se hace un store hay que pasar a ready el primer esi encolado en espera de esa clave
								//si es con desalojo y se desbloquea un esi, hay que replanificar

							} else { //otro esi tiene bloqueada la clave o no la tiene nadie
								header.context=abortESI;
								header.mSize=0;
								sendHead(coordinadorSocket,header);
							}
							break;
						default:
							printf("La solicitud del ESI %d es inválida.\n",idESI);
						}
					}
				} else { //Un ESI me quiere decir algo
					t_head header = recvHead(i);
					if (header.context == ERROR_HEAD) {
						close(i);
						FD_CLR(i, &master); // remove from master set
					} else {
						// we got some data from an ESI
						//char* clave = malloc(header.mSize); //esto no se por que lo habiamos puesto
						switch (header.context) {
						case blockedESI:
							//recv(i, clave, header.mSize, 0);
							printf("El ESI %d me informa que queda bloqueado esperando la clave %s.\n",running->idESI, paqueteGet.clave);
							hayQuePlanificar=true;
							proximoESI=NULL;
							running=NULL;
							break;
						case okESI:
							printf("El ESI %d finalizo su accion correctamente.\n",running->idESI);
							break;
						case terminatedESI:
							hayQuePlanificar=true;
							proximoESI=NULL;
							close(i);
							printf("El esi %d termino de correr su script. \n",running->idESI);
							finalizarESI(); //libera recursos y manda a lista de finalizados
							FD_CLR(i, &master);
							running=NULL;
							break;
						case abortESI:
							hayQuePlanificar=true;
							proximoESI=NULL;
							close(i);
							printf("El esi %d aborto. \n",running->idESI);
							finalizarESI(); //libera recursos y manda a lista de finalizados
							FD_CLR(i,&master);
							running=NULL;
							break;
						default:
							printf("Error en ESI.\n");
						}
						hayQueEnviarOrdenDeEjecucion=true;
					}
				}
			} // END got new incoming connection
		} // END looping through file descriptors


		if(hayQuePlanificar){
			printf("hayQuePlanificar==true\n");
			proximoESI=planificar();
			hayQuePlanificar=false;
		}
		else{
			printf("hayQuePlanificar==false\n");
		}

		if(hayQueEnviarOrdenDeEjecucion){
			enviarOrdenDeEjecucion();
			hayQueEnviarOrdenDeEjecucion=false;
		}

	} // END for(;;)--and you thought it would never end!

	close(listeningSocket);
	log_destroy(logPlanificador);
//	consola();

	return 0;
}

//************* FUNCIONES *******************

void agregarNuevoESIAColaDeListos(int socketESI, int id) {
	t_ESI *nuevoESI = malloc(sizeof(t_ESI));
	nuevoESI->idESI = id;
	nuevoESI->socket = socketESI;
	nuevoESI->estimado = ESTIMACION_I;
	nuevoESI->listoDesde = clock;

	list_add(listos, nuevoESI);
}

void agregarESIAColaDeListos(t_ESI *esi) {
	esi->estimado = (ALFA / 100) * (esi->real) + ((1 - (ALFA / 100)) * esi->estimado);
	esi->listoDesde = clock;
	esi->real=0;

	list_add(listos, esi);
}

//-------------------------------------------------
t_ESI *planificar() {
	t_ESI *esi;
	if(!list_is_empty(listos)){
		if (!strcmp(ALGORITMO, "SJF-SD")) {
			puts("se planifica con SJFSD"); //para comprobar
			esi = sjfsd();
		}
//		else if (!strcmp(ALGORITMO, "SJF-CD")) {
//			esi = sjfcd();
//		}
//		else if (!strcmp(ALGORITMO, "HRRN")) {
//			esi = hrrn();
//		}
		else {
			puts("Error: No se pudo determinar el algoritmo de planificación");
			return NULL;
		}
		return esi;
	}
	else{
		return NULL;
	}
}


t_ESI *sjfsd() {
	if (!list_is_empty(listos)) {
		t_ESI *esi;
		t_ESI *esiElegido;
		int index = 0;
		int indexDefinitivo;

		esiElegido = list_get(listos, index++);
		indexDefinitivo=index-1;

		while (index < list_size(listos)) {
			// Guarda la instancia de ese index y lo incrementa
			esi = list_get(listos, index++);

			if (esi->estimado < esiElegido->estimado){
				// Guarda la nueva instancia con mayor entradas libres como candidata
				esiElegido = esi;
				indexDefinitivo=index-1;
			}
		}
		list_remove(listos,indexDefinitivo);
		return esiElegido;
	} else {
		return NULL;
	}
}

void enviarOrdenDeEjecucion(){
	if(proximoESI!=NULL){ //se envia orden de ejecutar al esi guardado en la variable proximoESI
		t_head header;
		header.context=executeESI;
		header.mSize=0;
		sendHead(proximoESI->socket,header);
		clock++;
		(proximoESI->real)++;
		running=proximoESI;
	}
}

void bloquearESI(char* clave){

	if(dictionary_has_key(colasBloqueados, clave)){						//si ya hay procesos encolados
		queue_push(dictionary_get(colasBloqueados,clave),running);		//agregar a la cola
	}
	else{																//sino
		dictionary_put(colasBloqueados, clave, queue_create());			//crear cola en el diccionario
		queue_push(dictionary_get(colasBloqueados,clave),running);		//agregar esi a la cola
	}
}

void finalizarESI(){
	liberarRecursos();
	list_add(finalizados, running);
}

void liberarRecursos(){
	int table_index;
	for (table_index = 0; table_index < clavesBloqueadas->table_max_size; table_index++) {
		t_hash_element *element = clavesBloqueadas->elements[table_index];
		while (element != NULL) {

			if(*(int*)(element->data)==running->idESI){
				dictionary_remove(clavesBloqueadas,element->key);
			}

			element = element->next;
		}
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
