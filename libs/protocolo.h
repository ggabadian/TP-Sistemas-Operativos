
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>
#include <stdint.h>

char* identificar(int);
int recibirHead(int);
void enviarHead(int, int);


// HEAD sirve para elegir la accion a realizar
// se declaran en protocolo.h y se definen en protocolo.c

// -------- HEADs --------

	extern uint32_t COORDINADOR;
	extern uint32_t initDatosInstancia;

	extern uint32_t PLANIFICADOR;
//	extern uint32_t otraCosaDelPlanificador;

	extern uint32_t ESI;
//	extern uint32_t otraCosaDelESI;

	extern uint32_t INSTANCIA;
//	extern uint32_t otraCosaDeLaInstancia;

	// MENSAJES COMPARTIDOS
	extern uint32_t ERROR_HEAD;
	extern uint32_t ACT_GET;
	extern uint32_t ACT_SET;
	extern uint32_t ACT_STORE;

// -----------------------

// Estructura de paquetes en mensajes principales
typedef struct _t_Package {
	uint32_t head;
	char* message;
	uint32_t message_long;
} t_Package;

#endif /* LIBS_PROTOCOLO_H_ */
