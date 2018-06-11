#ifndef COORDINADOR_H_
#define COORDINADOR_H_

#include "configCoordinador.h"
#include "../../libs/socketServer.h"
#include "../../libs/protocolo.h"
#include <pthread.h>

void crearThread(int, int);
void threadPlanificador(void*);
void threadESI(void*);
void threadInstancia(void*);
void recibirMensaje(int);
void sendInitInstancia(int, int, int);

#endif
