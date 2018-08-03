#include "Planificador.h"

// Defino los nombres de las listas a utilizar
t_list *listos;
t_dictionary *colasBloqueados;
t_list *finalizados;
t_dictionary *clavesBloqueadas;

bool pausarPlanificador;
bool runningFinalizadoPorConsola;
bool runningBloqueadoPorConsola;

int systemClock;	//clock del sistema. Se incrementa cada vez que se envia una orden de ejecucion. (cuenta la cantidad total de ordenes enviadas
					//tambien sirve para medir el tiempo que un esi lleva esperando en la cola de listos. (debe guardarse en su t_esi y calcular la diferencia)
int idESI;			//Lo hago global ya que se utiliza en la consola

t_ESI *proximoESI;
t_ESI *running;

fd_set master;

int main(int argc, char* argv[]) {

	int statusConsola, statusMainProgram;
	pthread_t tConsola, tMainProgram;

	pausarPlanificador = false;

	runningFinalizadoPorConsola=false;
	runningBloqueadoPorConsola=false;


	// Creo las diferentes listas a ser utilizadas
	listos = list_create();
	colasBloqueados = dictionary_create(); //se entra por clave y devuelve cola de esis bloqueados en espera de la clave
	finalizados = list_create();
	clavesBloqueadas = dictionary_create(); //se entra por clave y devuelve el id del esi que la tiene

	//se usa para escribir en el archivo de log y lo muestra por pantalla
	logPlanificador = log_create("../logs/logDePlanificador.log","Planificador", false, LOG_LEVEL_TRACE);

	log_trace(logPlanificador, "\n\n\n\n\nIniciando Planificador\n\n\n\n");

	// Cargo la configuración del Planificador desde el archivo config
	cargarConfigPlanificador(argv[1]);

	log_info(logPlanificador, "PUERTO= %s", PUERTO);
	log_info(logPlanificador, "ALGORITMO= %s", ALGORITMO);
	log_info(logPlanificador, "ALFA= %d", ALFA);
	log_info(logPlanificador, "ESTIMACION= %d", ESTIMACION_I);
	log_info(logPlanificador, "IP COORDINADOR= %s", IP_COORDINADOR);
	log_info(logPlanificador, "PUERTO COORDINADOR= %s\n", PUERTO_COORDINADOR);

	//Agrego las claves bloqueadas por archivo configuración a la lista de claves

	int *idEsiConfig = malloc(sizeof(int));
	*idEsiConfig = -1;
	for (int claveI = 0; CL_BLOQUEADAS[claveI] != NULL; claveI++) {
		log_info(logPlanificador, "CLAVES BLOQUEADAS= %s", CL_BLOQUEADAS[claveI]);
		dictionary_put(clavesBloqueadas, CL_BLOQUEADAS[claveI], idEsiConfig);
	}

	//Creo los threads para la consola y el main Program
	statusConsola = pthread_create(&tConsola, NULL, &consola, NULL);
	if (statusConsola) {
		log_error(logPlanificador, "No se pudo crear el thread para la Consola\n");
	}
	if (!statusConsola) {
		log_info(logPlanificador, "Se crea thread para la consola\n");
	}

	statusMainProgram = pthread_create(&tMainProgram, NULL, &mainProgram, NULL);
	if (statusMainProgram) {
		log_error(logPlanificador, "No se pudo crear el thread para el main program\n");
	}
	if (!statusMainProgram) {
		log_info(logPlanificador, "Se crea thread para el main program\n");
	}

	pthread_join(tConsola, NULL);
	pthread_join(tMainProgram, NULL);


	return 0;
}

//************* FUNCIONES *******************

void *mainProgram() {

	bool hayQuePlanificar=false;
	log_info(logPlanificador,"Se seteó hayQuePlanificar=false\n");
	bool hayQueEnviarOrdenDeEjecucion=false;
	bool conDesalojo;

	systemClock=0;
	proximoESI=NULL;
	running=NULL;

	if (strcmp(ALGORITMO, "SJF-CD") == 0) {
		conDesalojo = true;
	}
	else{
		conDesalojo = false;
	}

	t_head headerAEnviar;
	headerAEnviar.context = PLANIFICADOR;
	headerAEnviar.mSize = 0;

	//Me conecto al Coordinador
	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR); //Envío solicitud de conexión al Coordinador
	log_info(logPlanificador,"Conectado a Coordinador. \n");
	sendHead(coordinadorSocket, headerAEnviar); // Le aviso al Coordinador que soy el Planificador

	//Creo socket para escuchar conexiones de ESIs entrantes
	int listeningSocket = listenSocket(PUERTO);
	listen(listeningSocket, BACKLOG);

	idESI = 0; // Cantidad de ESIs conectados

	//fd_set master; 		// master file descriptor list
	fd_set read_fds; 	// temp file descriptor list for select()
	int fdmax;
	FD_ZERO(&master);	// clear the master and temp sets
	FD_ZERO(&read_fds);

	// add the listener to the master set
	FD_SET(listeningSocket, &master);
	FD_SET(coordinadorSocket, &master);

	// keep track of the biggest file descriptor
	fdmax = (listeningSocket > coordinadorSocket ?listeningSocket : coordinadorSocket);

	struct timeval timeout;
//	timeout.tv_sec = 1;
//	timeout.tv_usec = 0;
	int valorSelect = 0;

	t_get paqueteGet;
	t_set paqueteSet;
	t_store paqueteStore;

	// main loop
	for (;;) {
		read_fds = master; // copy it
		timeout.tv_sec = 1;
		timeout.tv_usec = 0;
		valorSelect = select(fdmax + 1, &read_fds, NULL, NULL, &timeout);
		if ( valorSelect == -1) {
			perror("select");
			exit(4);
		} else if (valorSelect != 0) {

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
							log_error(logPlanificador,
									"Error en HANDSHAKE: No se pudo identificar a la entidad. Conexión desconocida.\n");
						} else {
							log_info(logPlanificador,"Conectado a %s.\n",identificar(identificador.context));
						}

						if (identificador.context == ESI) { // Si es un ESI, le asigna un id
							idESI++;
							send(socketCliente, &idESI, sizeof(idESI), 0);
							if(list_is_empty(listos) && running==NULL){
								hayQueEnviarOrdenDeEjecucion=true; //cuando se conecta el primer esi, si o si hay que planificar y enviar orden
								hayQuePlanificar=true;
								log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
							}
							agregarNuevoESIAColaDeListos(socketCliente, idESI); //Agrego el nuevo ESI a la cola de listos
							if (conDesalojo){
								hayQuePlanificar=true;
								log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
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
								log_info(logPlanificador,"Se recibió un GET <%s> del ESI %d.",paqueteGet.clave, paqueteGet.idESI);



								// verificar si la solicitud es valida
								if (dictionary_get(clavesBloqueadas,paqueteGet.clave) == NULL) { //nadie tiene tomada la clave
									uint32_t* a = malloc(sizeof(uint32_t));
									*a = paqueteGet.idESI;
									dictionary_put(clavesBloqueadas,paqueteGet.clave, a);
									header.context=okESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									log_info(logPlanificador,"Nadie tenia tomada la clave, se aprueba el GET.");
									log_info(logPlanificador,"Se entrego la clave %s al ESI %d.",paqueteGet.clave,*(int*)dictionary_get(clavesBloqueadas,paqueteGet.clave));

								} else if ((*(int*) (dictionary_get(clavesBloqueadas,paqueteGet.clave))) == paqueteGet.idESI) { //el esi que pide es el que tiene tomada la clave (la esta pidiendo por segunda vez
									header.context=okESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									log_info(logPlanificador,"El ESI ya tenia tomada la clave, se aprueba el GET.");

								} else { // otro esi tiene bloqueada la clave
									header.context=blockedESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									bloquearESI(paqueteGet.clave); //esto solo agrega a la cola de bloqueados
									log_info(logPlanificador,"Otro ESI tenia tomada la clave, se bloquea el ESI.");
									//hayQueplanificar se setea cuando el esi retorna
								}

								break;

							case OPERACION_SET:
								recvSet(i, &paqueteSet);
								log_info(logPlanificador,"Se recibió un SET <%s> <%s> del ESI %d.",
										paqueteSet.clave,
										paqueteSet.valor,
										paqueteSet.idESI);
								// verificar si la solicitud es valida
								if ((dictionary_get(clavesBloqueadas,paqueteSet.clave))!=NULL && *(int*) (dictionary_get(clavesBloqueadas,paqueteSet.clave)) == paqueteSet.idESI) { //el esi que pide es el que tiene tomada la clave
									header.context=okESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									log_info(logPlanificador,"El ESI ya tenia tomada la clave, se aprueba el SET.");
								} else { //otro esi tiene bloqueada la clave o no la tiene nadie
									header.context=abortESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									log_info(logPlanificador,"El ESI no tenia tomada la clave, se rechaza el SET y se aborta el ESI.");
								}

								//Usar donde corresponda:
								free(paqueteSet.valor);
								break;
							case OPERACION_STORE:
								recv(i, &paqueteStore, header.mSize, 0);
								log_info(logPlanificador,"Se recibió un STORE <%s> del ESI %d \n",
										paqueteStore.clave, paqueteStore.idESI);

								// verificar si la solicitud es valida
								if ((dictionary_get(clavesBloqueadas,paqueteStore.clave)!=NULL) && *(int*) (dictionary_get(clavesBloqueadas,paqueteStore.clave)) == paqueteStore.idESI) { //el esi que pide es el que tiene tomada la clave
									dictionary_remove_and_destroy(clavesBloqueadas,paqueteStore.clave,free);
									header.context=okESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
									bool seDesbloqueo = desbloquearDeCola(paqueteStore.clave); //cuando se hace un store hay que pasar a ready el primer esi encolado en espera de esa clave
									if(seDesbloqueo && conDesalojo){ //si es con desalojo y se desbloquea un esi, hay que replanificar
										hayQuePlanificar=true;
										log_info(logPlanificador,"Se seteó hayQuePlanificar=true");
									}
									log_info(logPlanificador,"El ESI ya tenia tomada la clave, se aprueba el STORE\n");
								} else { //otro esi tiene bloqueada la clave o no la tiene nadie
									log_info(logPlanificador,"El ESI no tenia tomada la clave, se rechaza el STORE y se aborta el ESI.\n");
									header.context=abortESI;
									header.mSize=0;
									sendHead(coordinadorSocket,header);
								}
							break;

							case ORDEN_COMPACTAR:
								//Bloquear consola, etc
								break;

							case FIN_COMPACTAR:
								//Desbloquear consola, etc
								break;

							default:
								log_info(logPlanificador,"La solicitud del ESI %d es inválida.\n",idESI);
							}
						}
					} else { //Un ESI me quiere decir algo
						t_head header = recvHead(i);
						if (header.context == ERROR_HEAD) {
							close(i);
							FD_CLR(i, &master); // remove from master set
							idESICaido=
							log_info(logPlanificador,"Se desconectó el ESI %d.\n",idESICaido);
						} else {
							// we got some data from an ESI
							//char* clave = malloc(header.mSize); //esto no se por que lo habiamos puesto
							switch (header.context) {
							case blockedESI:
								//recv(i, clave, header.mSize, 0);
								log_info(logPlanificador,"El ESI %d me informa que queda bloqueado esperando la clave %s.\n",running->idESI, paqueteGet.clave);
								hayQuePlanificar=true;
								log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
								proximoESI=NULL;
								running=NULL;
								break;
							case okESI:
								log_info(logPlanificador,"El ESI %d finalizo su accion correctamente.\n",running->idESI);
								break;
							case terminatedESI:
								hayQuePlanificar=true;
								log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
								proximoESI=NULL;
								close(i);
								log_info(logPlanificador,"El esi %d termino de correr su script. \n",running->idESI);
								finalizarESI(running); //libera recursos y manda a lista de finalizados
								FD_CLR(i, &master);
								running=NULL;
								break;
							case abortESI:
								proximoESI=NULL;
								close(i);
								log_info(logPlanificador,"Se abortó al ESI %d.\n",running->idESI);
								finalizarESI(running); //libera recursos y manda a lista de finalizados
								FD_CLR(i,&master);
								hayQuePlanificar=true;
								log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
								running=NULL;
								break;
							default:
								log_info(logPlanificador,"Error en ESI.\n");
							}
							hayQueEnviarOrdenDeEjecucion=true;
						}
					}
				} // END got new incoming connection
			} // END looping through file descriptors
		}

		if(runningFinalizadoPorConsola && hayQueEnviarOrdenDeEjecucion){
			finalizarESI(running);
			hayQuePlanificar=true;
			log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
			runningFinalizadoPorConsola=false;
			sleep(1);
		}

		if(runningBloqueadoPorConsola && hayQueEnviarOrdenDeEjecucion){
			bloquearESIConsola(running,claveGlobal);
			hayQuePlanificar=true;
			log_info(logPlanificador,"Se seteó hayQuePlanificar=true\n");
			runningBloqueadoPorConsola=false;
			free(claveGlobal);
			//running=NULL;
			proximoESI=NULL;
			sleep(1);
		}

		if(hayQuePlanificar && !pausarPlanificador){
			//log_info(logPlanificador,"hayQuePlanificar==true\n");
			proximoESI=planificar();
			if(proximoESI!=NULL){
				hayQuePlanificar=false;
				log_info(logPlanificador,"Se seteó hayQuePlanificar=false\n");
			}
		} else if(!pausarPlanificador) {
			//log_info(logPlanificador,"hayQuePlanificar==false\n");
		}

		if(hayQueEnviarOrdenDeEjecucion && !pausarPlanificador){
			if(proximoESI!=NULL){
				enviarOrdenDeEjecucion();
				hayQueEnviarOrdenDeEjecucion=false;
			}

		}

	} // END for(;;)--and you thought it would never end!

	close(listeningSocket);
	close(coordinadorSocket); //Según programa demo que ví se debe cerrar la conexión a pesar de ser clientes
	log_destroy(logPlanificador);


	return NULL;
}

void agregarNuevoESIAColaDeListos(int socketESI, int id) {
	t_ESI *nuevoESI = malloc(sizeof(t_ESI));
	nuevoESI->idESI = id;
	nuevoESI->socket = socketESI;
	nuevoESI->estimado = ESTIMACION_I;
	nuevoESI->remaining = ESTIMACION_I;
	log_info(logPlanificador,"La estimacion del (nuevo) esi %d es %f.",nuevoESI->idESI, nuevoESI->estimado);
	nuevoESI->listoDesde = systemClock;
	nuevoESI->real = 0;
	log_info(logPlanificador,"Se puso en 0 el real del esi %d. (nuevo ESI).\n",nuevoESI->idESI);

	list_add(listos, nuevoESI);
}

void agregarESIAColaDeListos(t_ESI *esi) {
	log_info(logPlanificador, "La rafaga anterior del esi %d duró %d",esi->idESI,esi->real);
	log_info(logPlanificador,"real: %d. EstimadoAnterior: %f. ALFA: %d",esi->real, esi->estimado, ALFA );
	esi->estimado = (float)(ALFA / 100.0) * (float)(esi->real) + (float)((1 - (ALFA / 100.0)) * esi->estimado);
	esi->remaining = esi->estimado;
	log_info(logPlanificador,"La nueva estimacion del (pasado a ready) esi %d es %f",esi->idESI, esi->estimado);
	esi->listoDesde = systemClock;
	esi->real = 0;
	log_info(logPlanificador,"Se puso en 0 el real del esi %d. (Pasado a listo).\n",esi->idESI);

	list_add(listos, esi);
}

void desalojar(t_ESI *esi) {
	esi->remaining = (float)esi->estimado-(float)esi->real;
	esi->listoDesde = systemClock;

	list_add(listos, esi);
}

t_ESI *planificar() {
	t_ESI *esi;
	if(!list_is_empty(listos)){
		if (!strcmp(ALGORITMO, "SJF-SD")) {
			log_info(logPlanificador,"Se planifica con SJF-SD."); //para comprobar
			esi = sjfsd();
		}
		else if (!strcmp(ALGORITMO, "SJF-CD")) {
			log_info(logPlanificador,"Se planifica con SJF-CD."); //para comprobar
			esi = sjfcd();
		}
		else if (!strcmp(ALGORITMO, "HRRN")) {
			log_info(logPlanificador,"Se planifica con HRRN."); //para comprobar
			esi = hrrn();
		}
		else {
			log_error(logPlanificador,"Error: No se pudo determinar el algoritmo de planificación");
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

			if (esi->remaining < esiElegido->remaining){
				// Guarda la nueva instancia con mayor entradas libres como candidata
				esiElegido = esi;
				indexDefinitivo=index-1;
			}
		}
		list_remove(listos,indexDefinitivo);
		log_info(logPlanificador,"Se eligio al ESI %d.\n",esiElegido->idESI);
		return esiElegido;
	} else {
		log_info(logPlanificador,"No fue posible planificar porque no hay ESIs en la cola de listos.\n");
		return NULL;
	}
}

t_ESI *sjfcd() {
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

			if (esi->remaining < esiElegido->remaining){
				// Guarda la nueva instancia con mayor entradas libres como candidata
				esiElegido = esi;
				indexDefinitivo=index-1;
			}
		}

		if(running!=NULL && (running->estimado - running->real)<esiElegido->remaining){
			log_info(logPlanificador,"El ESI %d conitnua ejecutando.\n",running->idESI);
			return running;
		}
		else{
			list_remove(listos,indexDefinitivo);
			if(running!=NULL){
				desalojar(running);
			}
			log_info(logPlanificador,"Se eligio al ESI %d.\n",esiElegido->idESI);
			return esiElegido;
		}

	} else {
		log_info(logPlanificador,"No fue posible planificar porque no hay ESIs en la cola de listos.\n");
		return NULL;
	}
}

t_ESI *hrrn() {
	if (!list_is_empty(listos)) {
		t_ESI *esi;
		t_ESI *esiElegido;
		int index = 0;
		int indexDefinitivo;
		float rr;
		float rrElegido;


		esiElegido = list_get(listos, index++);
		indexDefinitivo=index-1;
		rrElegido=getResponseRatio(esiElegido);

		while (index < list_size(listos)) {
			esi = list_get(listos, index++);
			rr = getResponseRatio(esi);
			if (rr > rrElegido){
				esiElegido = esi;
				rrElegido = rr;
				indexDefinitivo=index-1;
			}
		}

		list_remove(listos,indexDefinitivo);
			log_info(logPlanificador,"Se eligio al ESI %d.\n",esiElegido->idESI);
			return esiElegido;
		} else {
			log_info(logPlanificador,"No fue posible planificar porque no hay ESIs en la cola de listos.\n");
			return NULL;
		}
}

float getResponseRatio(t_ESI* esi){
	float estimado = esi->estimado;
	int espera = systemClock-esi->listoDesde;
	float rr = 1+espera/estimado;

	log_info(logPlanificador,"El RR del esi %d es %f", esi->idESI, rr);

	return rr;
}

void enviarOrdenDeEjecucion(){
	if(proximoESI!=NULL){ //se envia orden de ejecutar al esi guardado en la variable proximoESI
		log_info(logPlanificador,"Se envia la orden de ejecucion al ESI %d.",proximoESI->idESI);
		t_head header;
		header.context=executeESI;
		header.mSize=0;
		sendHead(proximoESI->socket,header);
		systemClock++;
		running=proximoESI;
		running->real+=1;
		log_info(logPlanificador,"Es la %d° operacion del ESI %d.",running->real,running->idESI);
	}
}

void bloquearESI(char* clave){

	if(!dictionary_has_key(colasBloqueados, clave)){					//si no hay procesos encolados
		dictionary_put(colasBloqueados, clave, queue_create());			//crear cola en el diccionario
	}
	queue_push((t_queue*)dictionary_get(colasBloqueados,clave),running);		//agregar esi a la cola
}

int desbloquearClave(char* clave){
	if (dictionary_has_key(colasBloqueados,clave)){
		t_ESI* procesoDesencolado = queue_pop((t_queue*)dictionary_get(colasBloqueados,clave));
		agregarESIAColaDeListos(procesoDesencolado);
		if(queue_is_empty((t_queue*)dictionary_get(colasBloqueados,clave))){
			queue_destroy((t_queue*)dictionary_get(colasBloqueados,clave));
			dictionary_remove(colasBloqueados,clave);
		}
		if ((int*)dictionary_get(clavesBloqueadas,clave)== NULL){
			printf("Nadie tenía tomada la clave\n");
		}
		else if(*(int*)dictionary_get(clavesBloqueadas,clave)==-1)
			dictionary_remove(clavesBloqueadas,clave);
		else
			dictionary_remove_and_destroy(clavesBloqueadas,clave,free);

		return procesoDesencolado->idESI;
	} else{
		return -1;
	}
}


bool desbloquearDeCola(char* clave){
	if (dictionary_has_key(colasBloqueados,clave)){
		t_ESI* procesoDesencolado = queue_pop((t_queue*)dictionary_get(colasBloqueados,clave));
		agregarESIAColaDeListos(procesoDesencolado);
		if(queue_is_empty((t_queue*)dictionary_get(colasBloqueados,clave))){
			queue_destroy((t_queue*)dictionary_get(colasBloqueados,clave));
			dictionary_remove(colasBloqueados,clave);
		}
		return true;
	}
	else{
		return false;
	}
}

void finalizarESI(t_ESI* esi){
	liberarRecursos(esi);
	list_add(finalizados, esi);
	close(esi->socket);
	FD_CLR(esi->socket,&master);
}

void liberarRecursos(t_ESI* esi){
	int table_index;
	int tableSize = clavesBloqueadas->table_max_size;
	for (table_index = 0; table_index < tableSize; table_index++) {
		t_hash_element *element = clavesBloqueadas->elements[table_index];
		while (element != NULL) {
			t_hash_element *nextElement = element->next;
			if(*(int*)(element->data)==esi->idESI){
				desbloquearDeCola(element->key);
				dictionary_remove_and_destroy(clavesBloqueadas,element->key,free);
			}
			element = nextElement;
		}
	}
}

void *consola() {

	bool salir = false;
	int opcion = 0;
	int eleccion = 0;
	t_head headerAEnviar;
	headerAEnviar.context = CONSOLA;
	headerAEnviar.mSize = 0;

	//Me conecto al Coordinador
	int coordinadorSocketConsola = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR); //Envío solicitud de conexión al Coordinador //Lo llamo distinto al global por las dudas
	sendHead(coordinadorSocketConsola, headerAEnviar); // Le aviso al Coordinador que soy la consola del Planificador
	log_trace(logPlanificador, "Consola conectada al Coordinador.\n");


	do {
		opcion = 0;
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
		scanf("%d", &opcion);

		char* clave=malloc(40);
		int id;

		switch (opcion) {
		case 1:
			eleccion = 0;
			puts("\n\nPAUSAR / CONTINUAR");
			printf("\nOprima '1' para Pausar o '2' para Continuar: ");
			scanf("%d", &eleccion);
			if (eleccion == 1) {
				pausarPlanificador = true;
				printf("\n\nSe eligió Pausar");
			}else if (eleccion == 2) {
				pausarPlanificador = false;
				printf("\n\nSe eligió Continuar");
			}
			else {
				printf("\n\nOpcion incorrecta");
			}
			printf("\n\npausarPlanificador = %d", pausarPlanificador);
			//printf("\n\nhayQueEnviarOrdenDeEjecucion = %d", hayQueEnviarOrdenDeEjecucion);
			break;

		case 2:
			puts("BLOQUEAR");
			printf("Inserte Clave: ");
			scanf("%s", clave);
			printf("Escriba el ID del proceso a bloquear: ");
			scanf("%d", &id);
			if(id>idESI){												//no existe el esi
				puts("No se puede bloquear el ESI porque no existe ESI con ese ID en el sistema");
			} else if(algun_esi_es_id(finalizados,id)){					//el esi ya finalizo
				puts("No se puede bloquear el ESI porque ya esta finalizado");
			} else if(algun_esi_es_id(listos,id)){						//el esi esta en cola de listos
				t_ESI* esiQuitado = remover_esi_con_id(listos,id);
				bloquearESIConsola(esiQuitado,clave);
				puts("El ESI se encontraba listo. Se bloquea");
			} else if(running!=NULL && id == running->idESI){			//el esi es el running
				proximoESI=NULL;
				runningBloqueadoPorConsola=true;
				claveGlobal=malloc(strlen(clave)+1);
				strcpy(claveGlobal,clave);
				puts("El ESI es el running. Se bloquea");
			}else {														//el esi esta bloqueado
				puts("No se puede bloquear el ESI porque ya se encuentra bloqueado");
			}
			break;

		case 3:
			puts("DESBLOQUEAR CLAVE");
			printf("Inserte Clave: ");
			scanf("%s", clave);
			int esi = desbloquearClave(clave);
			if (esi > 0){
				printf("Se agregó el ESI: %d a la cola de listos", esi);
			} else{
				printf("No quedan procesos bloqueados para la clave %s. Se libera la misma.", clave);
			}
			printf("\n");
			break;

		case 4:
			puts("LISTAR");
			printf("Inserte Clave: ");
			scanf("%s", clave);
			if(dictionary_has_key(clavesBloqueadas,clave)){
				if (*(int*)dictionary_get(clavesBloqueadas,clave) != -1)
					printf("\nEl ESI %d tiene tomada la clave.\n\n",*(int*)dictionary_get(clavesBloqueadas,clave));
				else
					printf("La clave se encuentra bloqueada por archivo de configuracion.\n\n");
			}
			else{
				printf("\nNadie tiene tomada la clave.\n\n");
			}
			if(dictionary_has_key(colasBloqueados,clave)){
				printf("ESIS ENCOLADOS:\n");
				t_queue *esisEncolados;
				esisEncolados=dictionary_get(colasBloqueados,clave);
				int n = queue_size(esisEncolados);
				int i;
				for(i=0; i<n; i++){
					t_ESI *esi;
					esi = (t_ESI*)queue_pop(esisEncolados);
					printf("%d) ESI: %d,\n",i+1,esi->idESI);
					queue_push(esisEncolados,esi);
				}
			}
			else{
				printf("No hay ESIS en espera de la clave %s.",clave);
			}
			printf("\n");
			break;

		case 5:
			puts("KILL");
			printf("Escriba el ID del proceso a finalizar: ");
			scanf("%d", &id);
			if(id>idESI){												//no existe el esi
				puts("No existe ESI con ese ID en el sistema");
			} else if(algun_esi_es_id(finalizados,id)){					//el esi ya finalizo
				puts("El ESI ya esta finalizado");
			} else if(algun_esi_es_id(listos,id)){						//el esi esta en cola de listos
				t_ESI* esiQuitado = remover_esi_con_id(listos,id);
				finalizarESI(esiQuitado);
			} else if(running!=NULL && id == running->idESI){			//el esi es el running
				proximoESI=NULL;
				runningFinalizadoPorConsola=true;
			}else {														//el esi esta bloqueado
				buscar_en_colas_y_remover(colasBloqueados,id);
			}
			break;

		case 6:
			puts("STATUS CLAVE");
			printf("Escriba la clave que desee consultar: ");
			scanf("%s", clave);

			headerAEnviar.context = statusClave;
			headerAEnviar.mSize = MAX_CLAVE;
			sendHead(coordinadorSocketConsola, headerAEnviar); // Le pido al Coordinador el comando Status Clave
			log_trace(logPlanificador, "Se envió al coordinador la solicitud del status de la clave %s.\n", clave);

			send(coordinadorSocketConsola,clave,headerAEnviar.mSize,0);

			recvStatus(coordinadorSocketConsola, clave);

			if(dictionary_has_key(colasBloqueados,clave)){
				printf("ESIS ENCOLADOS:\n");
				t_queue *esisEncolados;
				esisEncolados=dictionary_get(colasBloqueados,clave);
				int n = queue_size(esisEncolados);
				int i;
				for(i=0; i<n; i++){
					t_ESI *esi;
					esi = (t_ESI*)queue_pop(esisEncolados);
					printf("%d) ESI: %d,\n",i+1,esi->idESI);
					queue_push(esisEncolados,esi);
					}
				}
				else{
					printf("No hay ESIS en espera de la clave %s.",clave);
				}
				printf("\n");
			break;

		case 7:
			puts("Los deadlocks existentes son:\n");
			dictionary_iterator(colasBloqueados,analizarDeadlock);
			break;
		default:
			break;
		}
		free(clave);

		printf("\nPresione ENTER para realizar otra operacion de consola.\n");
		char enter = 0;
		while (enter != '\r' && enter != '\n') { enter = getchar(); }
		enter = 0;
		while (enter != '\r' && enter != '\n') { enter = getchar(); }



	}while(!salir);

	return NULL;
}

void analizarDeadlock (char* clave , void* colaGenerico){
	t_queue* cola = (t_queue*)colaGenerico;
	t_list* lista = cola->elements;
	buscarDeadlock(lista, clave);
}

void buscarDeadlock(t_list* lista, char* clave){
	void elementoDeadlock(void* esi){
		int id = ((t_ESI*)esi)->idESI;
		int esiTomador = *(int*)dictionary_get(clavesBloqueadas,clave);

		char* claveQueEsperaElEsiTomador = buscarEnColasYRetornar(esiTomador);

		if(claveQueEsperaElEsiTomador!=NULL && *(int*)dictionary_get(clavesBloqueadas,claveQueEsperaElEsiTomador)==id){
			printf("Hay deadlock entre los ESIs %d y %d.\n", esiTomador, id);
		}
	}
	list_iterate(lista, elementoDeadlock);
}

char* buscarEnColasYRetornar(int idBuscado){

	char* claveCopia=NULL;

	void buscarLaClaveQueEsperaElId (char* clave , void* cola){
		t_list *lista = ((t_queue*)cola)->elements;
		if (algun_esi_es_id(lista, idBuscado)){
			claveCopia=clave;
		}
	}

	dictionary_iterator(colasBloqueados,buscarLaClaveQueEsperaElId);
	return claveCopia;
}

void recvStatus(int socket, char* clave){

	int sizeARecibir;

	recv(socket, &sizeARecibir, 4, MSG_WAITALL);

	if(sizeARecibir!=0){
		char* valor = malloc(sizeARecibir);
		recv(socket, valor, sizeARecibir, MSG_WAITALL);
		printf("El valor de la clave %s es: %s.\n",clave, valor);
		free(valor);
	}
	else{
		printf("La clave %s no posee ningún valor.\n",clave);
	}

	recv(socket, &sizeARecibir, 4, MSG_WAITALL);
	int aux = sizeARecibir;

	if(sizeARecibir!=0){
			char* nombreInstancia = malloc(sizeARecibir);
			recv(socket, nombreInstancia, sizeARecibir, MSG_WAITALL);
			printf("La clave %s se encuentra en la instancia: %s.\n",clave, nombreInstancia);
			free(nombreInstancia);
	}
	else{
		printf("La clave %s no se encuentra en ninguna instancia.\n",clave);
	}

	recv(socket, &sizeARecibir, 4, MSG_WAITALL);

	if(sizeARecibir!=0){
		char* nombreInstancia = malloc(sizeARecibir);
		recv(socket, nombreInstancia, sizeARecibir, MSG_WAITALL);
		if(aux==0){
			printf("La clave %s se almacenaría en la instancia: %s.\n",clave, nombreInstancia);
		}
		free(nombreInstancia);
	}
	else{
		printf("No hay instancias conectadas. No se puede simular la distribución.\n");
	}
}

bool algun_esi_es_id(t_list *lista, int idBuscado){
  bool condicionEsi(void* esi){
	  int id = ((t_ESI*)esi)->idESI;
	  return id == idBuscado;
  }
  return list_any_satisfy(lista,condicionEsi);
}

void buscar_en_colas_y_remover(t_dictionary *diccionario, int idBuscado){

	char* claveCopia;

	void remover_de_cola_si_tiene_id (char* clave , void* cola){
		t_list *lista = ((t_queue*)cola)->elements;
		if (algun_esi_es_id(lista, idBuscado)){
			t_ESI* esiQuitado = remover_esi_con_id(lista, idBuscado);
			finalizarESI(esiQuitado);
			printf("Se finalizo el ESI %d \n", esiQuitado->idESI);

			claveCopia=strdup(clave);

		}

	}

	dictionary_iterator(diccionario,remover_de_cola_si_tiene_id);
	if(queue_is_empty((t_queue*)dictionary_get(diccionario,claveCopia))){
		queue_destroy((t_queue*)dictionary_get(diccionario,claveCopia));
		dictionary_remove(diccionario,claveCopia);
	}
	free(claveCopia);
}

t_ESI* remover_esi_con_id(t_list* lista, int idBuscado){
	bool condicionEsi(void* esi){
		  int id = ((t_ESI*)esi)->idESI;
		  return id == idBuscado;
	  }
	return list_remove_by_condition(lista,condicionEsi);
}

void bloquearESIConsola(t_ESI* esiQuitado,char* clave){
	if(dictionary_has_key(colasBloqueados, clave)){						//si ya hay procesos encolados
		queue_push(dictionary_get(colasBloqueados,clave),esiQuitado);	//agregar a la cola
	}
	else{																//sino
		dictionary_put(colasBloqueados, clave, queue_create());			//crear cola en el diccionario
		queue_push(dictionary_get(colasBloqueados,clave),esiQuitado);	//agregar esi a la cola
	}
}





