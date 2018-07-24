// PROTOCOLO DE COMUNICACION

// Los mensajes tienen la forma: [HEAD:DATO]

// HEAD es el primer mensaje que se envia/recibe
// POR CONVENCION, los HEAD son una estructura (t_head) compuesta por:
// context -> Es de tipo enum (e_context) y se usa para determinar la accion que se va a realizar
// mSize -> Es un entero que determina la cantidad de bytes de DATO que se deben leer
//			en el proximo recv()

// Puerto de escucha del Coordinador: 5000
// Puerto de escucha del Planificador: 5001

// Aclaracion: Donde aparezca "uint32_t", entender como "int"

#include "protocolo.h"

// Esto solamente es para el mensaje inicial en HANDSHAKE
// Ejemplo: "Conectado a Coordinador"
char* identificar(e_context id) {
	switch(id){
		case COORDINADOR:
			return "Coordinador";
			break;
		case PLANIFICADOR:
			return "Planificador";
			break;
		case CONSOLA:
			return "Consola";
			break;
		case ESI:
			return "ESI";
			break;
		case INSTANCIA:
			return "Instancia";
			break;
		default:
			return "Error";
	}
}

// Antes de recibir un mensaje se debe recibir el HEAD
t_head recvHead(int socket){
	t_head head;
	if ((recv(socket, &head, sizeof(t_head), MSG_WAITALL)) <= 0){
		head.context = ERROR_HEAD;
		head.mSize = 0;
		return head;
	} else {
		return head;
	}
}

// Antes de enviar un mensaje se debe enviar el HEAD
void sendHead(int socket, t_head head){
	send(socket, &head, sizeof(t_head), 0);
}

int recvSet(int socket, t_set *paqueteSet){
	int status;

	status = recv(socket, paqueteSet->clave, sizeof(paqueteSet->clave), MSG_WAITALL);
	if (!status) return 0;

	status = recv(socket, &(paqueteSet->sizeValor), sizeof(paqueteSet->sizeValor), MSG_WAITALL);
	if (!status) return 0;

	paqueteSet->valor = malloc(paqueteSet->sizeValor);
	status = recv(socket, paqueteSet->valor, paqueteSet->sizeValor, MSG_WAITALL);
	if (!status) return 0;

	status = recv(socket, &(paqueteSet->idESI), sizeof(paqueteSet->idESI), MSG_WAITALL);
	if (!status) return 0;

	return status;

}

void sendSet(int socket, t_set *paqueteSet){
	int packageSize = sizeof(paqueteSet->clave) + sizeof(paqueteSet->idESI) + sizeof(paqueteSet->sizeValor) + paqueteSet->sizeValor;
	char *serializedPackage = malloc(packageSize);
	int offset = 0;
	int sizeToSend;

	sizeToSend = sizeof(paqueteSet->clave);
	memcpy(serializedPackage + offset, &(paqueteSet->clave), sizeToSend);
	offset += sizeToSend;

	sizeToSend = sizeof(paqueteSet->sizeValor);
	memcpy(serializedPackage + offset, &(paqueteSet->sizeValor), sizeToSend);
	offset += sizeToSend;

	sizeToSend = paqueteSet->sizeValor;
	memcpy(serializedPackage + offset, paqueteSet->valor, sizeToSend);
	offset += sizeToSend;

	sizeToSend = sizeof(paqueteSet->idESI);
	memcpy(serializedPackage + offset, &(paqueteSet->idESI), sizeToSend);

	send(socket, serializedPackage, packageSize, 0);

	free(serializedPackage);

}
