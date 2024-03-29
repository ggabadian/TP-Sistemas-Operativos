#include "ESI.h"

int id = 0; //Este es el ID del ESI. Lo provee el Planificador en el handshake

int main(int argc, char** argv) {
	t_log* logESI;//puede que haya que ponerla global si se usa en alguna funcion
	//creo el logger
	logESI = log_create("../logs/logDeESI.log", "ESI", true, LOG_LEVEL_TRACE);
	//se usa para escribir en el archivo de log y lo muestra por pantalla
	log_trace(logESI, "Iniciando ESI");
	cargarConfig(argv[2]);

	t_head header;

	/*
	 1. Conectarse al planificador.
	 2. Handshake planificador (recibir ID)
	 3. Conectarse al Coordinador
	 4. Handshake Coordinador
	 5. Esperar orden de ejecucion del planificador
	 6.1 leer linea del script
	 .2 Parsear instruccion
	 7. Encviar al coordinador la orden de ejecucion
	 8. Recibir resultado por parte del coordinador
	 9. Transimitir resultado al planificador
	 10. Volver a 5.
	 */

	// 1,2. Conectarse al planificador y hacer handshake
	int planificadorSocket = conectarAlPlanificador();
	if (planificadorSocket < 0) {
		return planificadorSocket; //Finaliza el ESI
	}

	// 3,4. Conectarse al coordinador y hacer handshake
	int coordinadorSocket = conectarAlCoordinador();
	if (coordinadorSocket < 0) {
		return coordinadorSocket; //Finaliza el ESI
	}

	// abrir archivo

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_esi_operacion parsed;
	int numline = 0;

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	// 6.1 leer linea del script
	read = getline(&line, &len, fp);
	parsed = parse(line);
	numline++;

	t_get paqueteGet;
	memset(&paqueteGet, 0, sizeof(t_get)); //Inicializa toda la estrucutra

	t_set paqueteSet;

	t_store paqueteStore;
	memset(&paqueteStore, 0, sizeof(t_store)); //Inicializa toda la estrucutra

	while (read != -1) {

		// 5. esperar orden de ejecucion (trucho)
//		printf("Presione ENTER para parsear la próxima línea");
//		char enter = 0;
//		while (enter != '\r' && enter != '\n') {
//			enter = getchar();
//		}

		// 5. esperar orden de ejecucion (posta)
		header = recvHead(planificadorSocket); //recibo orden de ejecucion



		switch (header.context){

			case executeESI:
			puts(line);
			// 7. enviar al coordinador la opercion
			if (parsed.valido) {
				switch (parsed.keyword) {
				case GET:
					header.context = OPERACION_GET;
					header.mSize = sizeof(paqueteGet);
					//parsed.argumentos.GET.clave[39]='\0';
					if (strlen(parsed.argumentos.GET.clave) <= 40){ // Maximo permitido por consigna
						strcpy(paqueteGet.clave, parsed.argumentos.GET.clave);
					} else {
						printf("Error en GET: La clave en la línea %d supera los 40 caracteres.\n", numline);
						break;
					}

					paqueteGet.idESI = id;

					sendHead(coordinadorSocket, header);
					send(coordinadorSocket, &paqueteGet, sizeof(paqueteGet), 0);
					break;
				case SET:
					header.context = OPERACION_SET;
					header.mSize = 0;

					if (strlen(parsed.argumentos.SET.clave) <= 40){ // Maximo permitido por consigna
						strcpy(paqueteSet.clave, parsed.argumentos.SET.clave);
					} else {
						printf("Error en SET: La clave en la línea %d supera los 40 caracteres.\n", numline);
						break;
					}

					paqueteSet.sizeValor = strlen(parsed.argumentos.SET.valor) + 1;

					paqueteSet.valor = malloc(paqueteSet.sizeValor);
					strncpy(paqueteSet.valor, parsed.argumentos.SET.valor, strlen(parsed.argumentos.SET.valor));
					(paqueteSet.valor)[strlen(parsed.argumentos.SET.valor)] = '\0';

					paqueteSet.idESI = id;

					sendHead(coordinadorSocket, header);
					sendSet(coordinadorSocket, &paqueteSet);
					free(paqueteSet.valor);
					break;
				case STORE:
					header.context = OPERACION_STORE;
					header.mSize = sizeof(paqueteStore);

					if (strlen(parsed.argumentos.STORE.clave) <= 40){ // Maximo permitido por consigna
						strcpy(paqueteStore.clave, parsed.argumentos.STORE.clave);
					} else {
						printf("Error en STORE: La clave en la línea %d supera los 40 caracteres.\n", numline);
						break;
					}

					paqueteStore.idESI = id;

					sendHead(coordinadorSocket, header);
					send(coordinadorSocket, &paqueteStore, sizeof(paqueteStore), 0);
					break;
				default:
					fprintf(stderr, "No se pudo interpretar \n");
					exit(EXIT_FAILURE);
				}
			} else {
				fprintf(stderr, "La linea no es valida\n");
				exit(EXIT_FAILURE);
			}

			header = recvHead(coordinadorSocket);	// 8. Recibir resultado por parte del coordinador
			sendHead(planificadorSocket, header);	// 9. Transimitir resultado al planificador
			switch (header.context){
				case blockedESI:
					continue;
				case okESI:
					break;
				case abortESI:
					puts("ESI abortado por orden del coordinador");
					exit(EXIT_FAILURE);
				default:
					puts("ESI abortado. No se reconoce la respuesta del coordinador");
					exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
			read = getline(&line, &len, fp); // 6. parseo de nuevo
			parsed = parse(line);
			numline++;
			break;

		case ERROR_HEAD:
			puts("EL Planificador me cerro la conexion");
			exit(EXIT_FAILURE);
			break;

		default:
			break;
		}
	}

	header.context=terminatedESI;
	header.mSize=0;
	sendHead(planificadorSocket,header);

	fclose(fp);
	if (line)
		free(line);

	close(coordinadorSocket);
	close(planificadorSocket);
	log_destroy(logESI);
	return 0;
}

int conectarAlPlanificador() {
	t_head header;
	header.context = ESI;
	header.mSize = 0;

	int planificadorSocket = connectSocket(IP_PLANIFICADOR,
			PUERTO_PLANIFICADOR);
	if (planificadorSocket < 0) {
		printf("Error al conectar al Planificador. \n");
		return planificadorSocket;
	} else {
		printf("Conectado a Planificador. \n");
	}
	sendHead(planificadorSocket, header); // Le avisa que es un ESI
	int status = recv(planificadorSocket, &id, sizeof(id), 0); // Recibe el id asignado por el Planificador
	if (status < 0) {
		puts("ERROR: El planificador no pudo asignar un id.\n");
		return status;
	} else {
		printf("Este es el ESI %d.\n", id);
	}
	return planificadorSocket;
}

int conectarAlCoordinador() {
	t_head header;
	header.context = ESI;
	header.mSize = 0;

	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);

	if (coordinadorSocket < 0) {
		printf("Error al conectar al Coordinador. \n");
		return coordinadorSocket;
	} else {
		printf("Conectado a Coordinador. \n");
	}
	sendHead(coordinadorSocket, header); // Le avisa que es un ESI
	send(coordinadorSocket, &id, sizeof(int), 0); // Le envia su id
	return coordinadorSocket;
}
