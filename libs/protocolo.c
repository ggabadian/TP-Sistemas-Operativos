// PROTOCOLO DE COMUNICACION

// Los mensajes tienen la estructura: [HEAD:N:DATO]
// HEAD son los primeros bytes que recibe una entidad
// POR CONVENCION, los HEAD tienen la estructura ABCD donde
// 		A (1 a 4)		-> 1 es para mensajes que espera el COORDINADOR
// 						-> 2 para el PLANIFICADOR
// 						-> 3 para el ESI
// 						-> 4 para la INSTANCIA
//     BCD (000 a 999)	-> Cada uno elige que valor darle a cada accion

// N es el numero que identifica la cantidad de bytes que debe recibir de DATO
// DATO es el mensaje posta y es lo necesario para completar la accion elegida
// segun el HEAD

#include "protocolo.h"

// HEAD sirve para elegir la accion a realizar
enum Head {
	initDataInstancia = 4000,
	ejemploCoordinador = 1001,
	otroEjemploESI = 3020,
};

// Para HANDSHAKE
char* identificar(char* id) {
	if (!strcmp(id, COORDINADOR)){
		return "Coordinador";
	}
	else if (!strcmp(id, PLANIFICADOR)) {
		return "Planificador";
	}
	else if (!strcmp(id, ESI)) {
		return "ESI";
	}
	else if (!strcmp(id, INSTANCIA)) {
		return "Instancia";
	}
	else {
		return "Error";
	}
}

