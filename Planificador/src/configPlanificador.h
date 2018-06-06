
#ifndef PLANIFICADOR_SRC_CONFIGPLANIFICADOR_H_
#define PLANIFICADOR_SRC_CONFIGPLANIFICADOR_H_

#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/string.h>

// Campos del planificador.config
char *PUERTO;
char *ALGORITMO;
int ALFA;
int ESTIMACION_I;
char* IP_COORDINADOR;
int PUERTO_COORDINADOR;
char** CL_BLOQUEADAS;
// -----------------------------

void cargarConfigPlanificador();

// Ubicacion del archivo planificador.config
#define PATH_PLANIFICADOR_CONFIG "/home/utnso/workspace/tp-2018-1c-Microblando-Ventanas/Planificador/planificador.config"


#endif /* PLANIFICADOR_SRC_CONFIGPLANIFICADOR_H_ */
