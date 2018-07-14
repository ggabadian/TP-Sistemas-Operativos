
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// -------- Contexto --------
typedef enum {
	COORDINADOR,
	INIT_INSTANCIA,

	PLANIFICADOR,
	//otraCosaDelPlanificador,

	ESI,
	blockedESI,
	okESI,
	abortESI,

	INSTANCIA,
	nombreInstancia,
	okInstancia,
	ORDEN_COMPACTAR,
	//errores de la instancia

	// MENSAJES COMPARTIDOS
	ERROR_HEAD,
	OPERACION_GET,
	OPERACION_SET,
	OPERACION_STORE
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
	char clave[40]; // Por consigna: Las claves son de hasta 40 caracteres
	uint32_t idESI;
} __attribute__((packed)) t_get;

typedef struct {
	char clave[40];
	uint32_t sizeValor;
	char* valor;
	uint32_t idESI;
} t_set;

typedef struct {
	char clave[40];
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
