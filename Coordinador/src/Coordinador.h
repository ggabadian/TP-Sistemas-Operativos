#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include "configCoordinador.c"
#include "../../libs/socketServer.c"
#include "../../libs/protocolo.c"
#include <pthread.h>

void threadESI(void*);
void recibirMensaje(int);

#endif
