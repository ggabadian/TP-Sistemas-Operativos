
#ifndef LIBS_PROTOCOLO_H_
#define LIBS_PROTOCOLO_H_

#include <sys/socket.h>
#include <stdint.h>

// -------- Contexto --------
typedef enum {
	COORDINADOR,
	initDatosInstancia,

	PLANIFICADOR,
	//otraCosaDelPlanificador,

	ESI,
	//otraCosaDelESI,

	INSTANCIA,
	//otraCosaDeLaInstancia,

	// MENSAJES COMPARTIDOS
	ERROR_HEAD,
	ACT_GET,
	ACT_SET,
	ACT_STORE
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
	uint32_t sizeClave;
	char clave[40]; // Por consigna: Las claves son de hasta 40 caracteres
	uint32_t sizeValor;
	char valor[255]; // Limitamos los caracteres para usar serializacion estatica
} __attribute__((packed)) t_set;

// --------------------------------------------------

char* identificar(e_context);
t_head recvHead(int);
void sendHead(int, t_head);

#endif /* LIBS_PROTOCOLO_H_ */
