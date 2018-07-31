#ifndef INSTANCIA_SRC_CONFIGINSTANCIA_H_
#define INSTANCIA_SRC_CONFIGINSTANCIA_H_

#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <commons/string.h>
#include <commons/log.h>
#include <commons/config.h>
#include <commons/bitarray.h>
#include <commons/collections/list.h>
#include <math.h>
#include <pthread.h>
#include <string.h>

// Campos de instancia.config
char *IP_COORDINADOR;
char *PUERTO_COORDINADOR;
char *ALGORITMO;
char *PUNTO_MONTAJE;
char *NOMBRE;
int INTERVALO_DUMP;
// -----------------------------

void cargarConfig();

// Ubicacion del archivo instancia.config
#define PATH_INSTANCIA_CONFIG "/home/utnso/workspace/tp-2018-1c-Microblando-Ventanas/Instancia/instancia.config"

#endif /* INSTANCIA_SRC_CONFIGINSTANCIA_H_ */
