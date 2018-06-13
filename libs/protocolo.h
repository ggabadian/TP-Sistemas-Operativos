
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>
#include <stdint.h>

#define MAX_HEAD_SIZE 4 // Por ser int

char* identificar(int);
int recibirHead(int);
void enviarHead(int, int);


// HEAD sirve para elegir la accion a realizar
// se declaran en protocolo.h y se definen en protocolo.c

// -------- HEADs --------

	extern uint32_t COORDINADOR;
	extern uint32_t initDatosInstancia;

	extern uint32_t PLANIFICADOR;
//	otraCosaDelPlanificador = 201,

	extern uint32_t ESI;
//	otraCosaDelESI = 301,

	extern uint32_t INSTANCIA;
//	otraCosaDeLaInstancia = 401,

	extern uint32_t ERROR_HEAD;

// -----------------------

// Estructura de paquetes en mensajes principales
typedef struct _t_Package {
	char head[MAX_HEAD_SIZE];
	char* message;
	uint32_t message_long;
} t_Package;

#endif /* LIBS_PROTOCOLO_H_ */
