#include "ESI.h"

int id = 0; //Este es el ID del ESI. Lo provee el Planificador en el handshake

int main(int argc, char** argv) {
	puts("Iniciando ESI...");
	cargarConfig();

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

	enviarHead(coordinadorSocket, ACT_GET);

	FILE * fp;
	char * line = NULL;
	size_t len = 0;
	ssize_t read;
	t_esi_operacion parsed;

	fp = fopen(argv[1], "r");
	if (fp == NULL) {
		perror("Error al abrir el archivo: ");
		exit(EXIT_FAILURE);
	}

	// 6.1 leer linea del script
	read = getline(&line, &len, fp);
	parsed = parse(line);

	//send(coordinadorSocket, &ACT_GET, sizeof(ACT_GET),0);
	while (read != -1) {
		// 5. esperar orden de ejecucion
		printf("Presione ENTER para finalizar el ESI\n");
		char enter = 0;
		while (enter != '\r' && enter != '\n') {
			enter = getchar();
		}

		if (parsed.valido) {
			switch (parsed.keyword) {
			case GET:
				printf("GET\tclave: <%s>\n", parsed.argumentos.GET.clave);
				//empaquetar
				//enviar paquete de GET al coordinador
				break;
			case SET:
				printf("SET\tclave: <%s>\tvalor: <%s>\n",
						parsed.argumentos.SET.clave,
						parsed.argumentos.SET.valor);
				//empaquetar
				//enviar paquete de SET al coordinador
				break;
			case STORE:
				printf("STORE\tclave: <%s>\n", parsed.argumentos.STORE.clave);
				break;
				//empaquetar
				//enviar paquete de STORE al coordinador
			default:
				fprintf(stderr, "No pude interpretar \n");
				exit(EXIT_FAILURE);
			}

			destruir_operacion(parsed);
		} else {
			fprintf(stderr, "La linea no es valida\n");
			exit(EXIT_FAILURE);
		}

		// 7. enviar al coordinador la opercion

		// 8. Recibir resultado por parte del coordinador

		// 9. Transimitir resultado al planificador

		/*
		 * if (me bloquee)
		 * 		continue
		 */

		read = getline(&line, &len, fp); // 6. parseo de nuevo
		parsed = parse(line);
	}

	fclose(fp);
	if (line)
		free(line);

	close(coordinadorSocket);
	close(planificadorSocket);

	return 0;
}

int conectarAlPlanificador() {
	int planificadorSocket = connectSocket(IP_PLANIFICADOR,
			PUERTO_PLANIFICADOR);
	if (planificadorSocket < 0) {
		printf("Error al conectar al Planificador. \n");
		return planificadorSocket;
	} else {
		printf("Conectado a Planificador. \n");
	}
	send(planificadorSocket, &ESI, 4, 0); // Le avisa que es un ESI
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
	int coordinadorSocket = connectSocket(IP_COORDINADOR, PUERTO_COORDINADOR);

	if (coordinadorSocket < 0) {
		printf("Error al conectar al Coordinador. \n");
		return coordinadorSocket;
	} else {
		printf("Conectado a Coordinador. \n");
	}
	send(coordinadorSocket, &ESI, 4, 0); // Le avisa que es un ESI
	return coordinadorSocket;
}
