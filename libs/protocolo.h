
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>

char* identificar(int);
int recibirHead(int);
void enviarHead(int, int);


// HEAD sirve para elegir la accion a realizar
// y para saber cuantos bytes tiene que recibir de DATO
// se declaran en protocolo.h y se definen en protocolo.c

// -------- HEADs --------

	extern int COORDINADOR;
	extern int initDatosInstancia;

	extern int PLANIFICADOR;
//	otraCosaDelPlanificador = 201,

	extern int ESI;
//	otraCosaDelESI = 301,

	extern int INSTANCIA;
//	otraCosaDeLaInstancia = 401,

	extern int ERROR_HEAD;

// -----------------------

#endif /* LIBS_PROTOCOLO_H_ */
