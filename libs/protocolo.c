// PROTOCOLO DE COMUNICACION

// Los mensajes tienen la estructura: [HEAD:DATO]

// HEAD son los primeros 4 bytes(por ser int) que recibe una entidad
// POR CONVENCION, los HEAD tienen la estructura ABC, donde:

// "A" va desde 1 hasta 4
// Los mensajes que empiezan con:
// A = 1	-> son del COORDINADOR
// A = 2	-> son del PLANIFICADOR
// A = 3	-> son del ESI
// A = 4	-> son de la INSTANCIA

// "BC" va desde 00 hasta 99
// BC = 00		-> Se usa para HANDSHAKE (100, 200, 300 y 400)
// BC (01 a 99)	-> Libres

// ACLARACIONES
// El COORDINADOR envia mensajes que empiezan con 100 y pico (ejemplo: 112)
// El PLANIFICADOR envia mensajes que empiezan con 200 y pico (ejemplo: 205)
// Y asi con todos

// DATO es lo necesario para completar la accion elegida segun el HEAD.
// En algunos casos podria no existir el DATO (Ejemplo: HANDSHAKE)

// Puerto de escucha del Coordinador: 5000
// Puerto de escucha del Planificador: 5001

#include "protocolo.h"


// HEAD sirve para elegir la accion a realizar
// se declaran en protocolo.h y se definen en protocolo.c

// Se usa el tipo uint32_t para que sean 4 bytes independientemente de la arquitectura

// -------- HEADs --------

	uint32_t COORDINADOR = 100;
	uint32_t initDatosInstancia = 101;

	uint32_t PLANIFICADOR = 200;
	//otraCosaDelPlanificador = 201,

	uint32_t ESI = 300;
	//otraCosaDelESI = 301,

	uint32_t INSTANCIA = 400;
	//otraCosaDeLaInstancia = 401,

	uint32_t ERROR_HEAD = 500;

// -----------------------


// Esto solamente es para el mensaje inicial en HANDSHAKE
// Ejemplo: "Conectado a Coordinador"
char* identificar(int id) {
	if (id==COORDINADOR){
		return "Coordinador";
	}
	else if (id==PLANIFICADOR) {
		return "Planificador";
	}
	else if (id==ESI) {
		return "ESI";
	}
	else if (id==INSTANCIA) {
		return "Instancia";
	}
	else {
		return "Error";
	}
}

// Antes de recibir un mensaje se debe recibir el HEAD
int recibirHead(int socket){
	int head = 0;
	if ((recv(socket, &head, 4, 0)) < 0){
		return ERROR_HEAD;
	} else {
		return head;
	}
}

// Antes de enviar un mensaje se debe enviar el HEAD
void enviarHead(int socket, int head){
	send(socket, &head, 4, 0);
}
