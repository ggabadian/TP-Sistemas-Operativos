
#ifndef PLANIFICADOR_H_
#define PLANIFICADOR_H_


#include "configPlanificador.h"
#include "../../libs/socketServer.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"
#include <pthread.h>
#include <commons/collections/list.h>
#include <commons/collections/queue.h>
#include <commons/collections/dictionary.h>

#define BACKLOG 5				//(Pendiente) Carga desde config
#define PACKAGESIZE 1024

//********** VARIABLES **********

t_log* logPlanificador; //Creo el logger



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

void *consola();
void *mainProgram();
void agregarNuevoESIAColaDeListos(int, int);
t_ESI *planificar ();
t_ESI *sjfsd();
void estimar();
void enviarOrdenDeEjecucion();
void bloquearESI(char*);
void finalizarESI();
void liberarRecursos();





#endif
