#ifndef ESI_SRC_CONFIGESI_H_
#define ESI_SRC_CONFIGESI_H_

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

// Campos del ESI.config
char *IP_COORDINADOR;
char *PUERTO_COORDINADOR;
char *IP_PLANIFICADOR;
char *PUERTO_PLANIFICADOR;

// -----------------------------

void cargarConfig(char*);

// Ubicacion del archivo coordinador.config
#define PATH_ESI_CONFIG "/home/utnso/workspace/tp-2018-1c-Microblando-Ventanas/ESI/ESI.config"

#endif /* ESI_SRC_CONFIGESI_H_ */
