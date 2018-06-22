
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_


#include "configPlanificador.h"
#include "../../libs/socketServer.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"
#include <commons/collections/list.h>


#define BACKLOG 5				//(Pendiente) Carga desde config
#define PACKAGESIZE 1024

//********** ESTRUCTURAS **********

typedef struct {
	int idESI;
	int socket;
	char **clTomadas;
	char *esperaClave;
	int tn;
	float estAntRaf;
	float estSigRaf;
} __attribute__((packed)) t_ESI;

typedef struct {
	int idEsi;
	char clTomada[40];
} __attribute__((packed)) t_clBloq;

//*********************************

//********** FUNCIONES **********

void consola();
void agregarESIAColaDeListos(int, int);



#endif
