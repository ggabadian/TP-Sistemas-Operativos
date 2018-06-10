#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include "configCoordinador.c"
#include "../../libs/socketServer.c"
#include "../../libs/protocolo.c"
#include <pthread.h>

void crearThread(int, int);
void threadPlanificador(void*);
void threadESI(void*);
void threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int, int, int);

#endif
