#ifndef ESI_H_
#define ESI_H_

#include "configESI.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"
#include <parsi/parser.h>

int conectarAlPlanificador();
int conectarAlCoordinador();


#define PACKAGESIZE 1024

#endif
