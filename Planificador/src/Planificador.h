
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
char* claveGlobal;



//********** ESTRUCTURAS **********

typedef struct {
	int idESI;
	int socket;
	int listoDesde;
	float estimado;  //no hace falta guardar el anterior y el proximo, cuando se calcula se pisa el anterior con el proximo
	int real;
	float remaining;
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
void agregarESIAColaDeListos(t_ESI*);
void desalojar(t_ESI*);
t_ESI *planificar ();
t_ESI *sjfsd();
t_ESI *sjfcd();
t_ESI *hrrn();
void estimar();
void enviarOrdenDeEjecucion();
void bloquearESI(char*);
void bloquearESIConsola(t_ESI* ,char*);
void finalizarESI();
void liberarRecursos();
int desbloquearClave(char*);
bool desbloquearDeCola(char*);
float getResponseRatio(t_ESI*);
bool algun_esi_es_id(t_list*, int);
t_ESI* remover_esi_con_id(t_list*, int);
void buscar_en_colas_y_remover(t_dictionary*, int);
void recvStatus(int, char*);
void remover_de_cola_si_tiene_id (char*, void*);
void analizarDeadlock(char*, void*);
void buscarDeadlock(t_list*, char*);
char* buscarEnColasYRetornar(int);





#endif
