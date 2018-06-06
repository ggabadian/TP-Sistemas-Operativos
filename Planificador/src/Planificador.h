
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_



#include "../../libs/socketServer.c"
#include "../../libs/socketClient.c"
#include "configPlanificador.c"

void consola();

char* PUERTO = "5000";
#define BACKLOG 5				//(Pendiente) Carga desde config
#define PACKAGESIZE 1024


#endif
