
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CLAVE 40

// -------- Contexto --------
typedef enum {
	COORDINADOR,
	INIT_INSTANCIA,
	REINCORPORACION_INSTANCIA,

	PLANIFICADOR,
	executeESI,

	CONSOLA,
	statusClave,
	statusValor,
	okRecibido, //Este header es sólo para probar la comunicación. Despúes deaparece.
	cerrarConexion, //Este header es sólo para probar la comunicación. Despúes deaparece.

	ESI,
	blockedESI,
	okESI,
	abortESI,
	terminatedESI,

	INSTANCIA,
	nombreInstancia,
	okInstancia,
	ORDEN_COMPACTAR,
	FIN_COMPACTAR,
	NRO_ENTRADAS,
	//errores de la instancia

	// MENSAJES COMPARTIDOS
	ERROR_HEAD,
	OPERACION_GET,
	OPERACION_SET,
	OPERACION_STORE,
} e_context;
// -----------------------

// -------- HEAD --------
typedef struct {
	e_context context; // Contexto
	uint32_t mSize; // Cantidad de bytes del DATO a recibir
} __attribute__((packed)) t_head;
// -----------------------

// ------------ ESTRUCTURAS COMPARTIDAS -------------

typedef struct {
	char clave[MAX_CLAVE]; // Por consigna: Las claves son de hasta 40 caracteres
	uint32_t idESI;
} __attribute__((packed)) t_get;

typedef struct {
	char clave[MAX_CLAVE];
	uint32_t sizeValor;
	char* valor;
	uint32_t idESI;
} t_set;

typedef struct {
	char clave[MAX_CLAVE];
	uint32_t idESI;
} __attribute__((packed)) t_store;



typedef struct {
	uint32_t cantidadEntradas;
	uint32_t sizeofEntrada;
} __attribute__((packed)) t_initInstancia;

// --------------------------------------------------

char* identificar(e_context);
t_head recvHead(int);
void sendHead(int, t_head);
int recvSet(int, t_set *);
void sendSet(int, t_set *);

#endif /* LIBS_PROTOCOLO_H_ */
