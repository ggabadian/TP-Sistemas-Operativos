
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
	int listoDesde;
	float estimado;  //no hace falta guardar el anterior y el proximo, cuando se calcula se pisa el anterior con el proximo
	float real;
} __attribute__((packed)) t_ESI;

typedef struct {
	int idEsi;
	char clTomada[40];
} __attribute__((packed)) t_clBloq;

//*********************************

//********** FUNCIONES **********

void consola();
void agregarNuevoESIAColaDeListos(int, int);
t_ESI *planificar ();
t_ESI *sjfsd();
void estimar();
void enviarOrdenDeEjecucion();
void okPermitirEjecutarEsi(int, t_head);




#endif
