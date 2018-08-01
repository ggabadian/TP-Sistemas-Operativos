#ifndef COORDINADOR_SRC_CONFIGCOORDINADOR_H_
#define COORDINADOR_SRC_CONFIGCOORDINADOR_H_

#include <string.h>
#include <commons/config.h>
#include <commons/string.h>

// Campos del coordinador.config
char *PUERTO;
char *ALGORITMO;
int CANTIDAD_ENTRADAS;
int BYTES_ENTRADA;
int RETARDO;
// -----------------------------

void cargarConfig(char*);

// Ubicacion del archivo coordinador.config
#define PATH_COORDINADOR_CONFIG "/home/utnso/workspace/tp-2018-1c-Microblando-Ventanas/Coordinador/coordinador.config"

#endif /* COORDINADOR_SRC_CONFIGCOORDINADOR_H_ */
