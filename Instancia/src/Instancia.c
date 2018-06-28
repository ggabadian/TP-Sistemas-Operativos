#include "Instancia.h"
#include <string.h>

int cantidadEntradasNecesarias(t_set *paqueteSet){
	double cantidadEntradas = paqueteSet->sizeValor/TAMANIO_ENTRADAS;
	return (int)ceil(cantidadEntradas);
}

int posicionEnTabla(char* clave, int *entradasUsadas){

}

void modificarLista(t_set *paqueteSet){
	datoEntrada *entrada;
	int cantidadEntradasUsadas;
	int cantidadEntradas = cantidadEntradasNecesarias(paqueteSet);
	int posicion = posicionEnTabla(paqueteSet->clave, &cantidadEntradasUsadas);
	if(posicion != 0){   //esta en la lista
		entrada = list_get(TABLA_ENTRADAS, posicion);

	}else{   // no esta en la lista

	}

}

void set(t_set *paqueteSet){
	datoEntrada *entrada;
	//busco en la tabla a ver si esta
	modificarLista(paqueteSet);


	if (TABLA_ENTRADAS->elements_count <= CANTIDAD_ENTRADAS){
	/*	uint32_t tamanio = paqueteSet->sizeValor;
		char* valor = paqueteSet->valor;
		char* clave = paqueteSet->clave;
		if(tamanio <= TAMANIO_ENTRADAS){
			entrada->tamanio;
			entrada->clave;
			agregarALista()
		}
		agregarALista(entrada);
		list_add(TABLA_ENTRADAS, );*/
	}else{//llene la lista
		if (strcmp(ALGORITMO, "CIRC")){

		}
		if (strcmp(ALGORITMO, "LRU")){

		}
		if (strcmp(ALGORITMO, "BSU")){

		}

	}

}
void store(t_store *paqueteStore){

}

bool enviarACoordinador(char* msg){
	int nAEnviar= strlen(msg);
	int nEnviados = send(SOCKET_COORDINADOR, msg, nAEnviar, 0);

	if(nAEnviar == nEnviados){
		return true;
	}
	return false;
}

bool conectarCoordinador(){
	t_head header;
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");

	SOCKET_COORDINADOR = connectSocket(IP_COORDINADOR,PUERTO_COORDINADOR);

	// Le avisa que es una INSTANCIA
	header.context = INSTANCIA;
	header.mSize = 0;
	sendHead(SOCKET_COORDINADOR, header);

	// Le envía su nombre
	header.context = nombreInstancia;
	header.mSize = strlen(NOMBRE);
	sendHead(SOCKET_COORDINADOR, header);
	enviarACoordinador(NOMBRE);

	return true;
}

bool recibirDeCoordinador(){
	t_head header = recvHead(SOCKET_COORDINADOR);
	t_set *paqueteSet= malloc(sizeof(t_set));
	t_store *paqueteStore= malloc(sizeof(t_store));
	t_initInstancia *paqueteInit= malloc(sizeof(t_initInstancia));

	switch(header.context){
	case INIT_INSTANCIA:
		recv(SOCKET_COORDINADOR, paqueteInit, header.mSize,0);
		CANTIDAD_ENTRADAS = paqueteInit->cantidadEntradas;
		TAMANIO_ENTRADAS = paqueteInit->sizeofEntrada;
		TABLA_ENTRADAS = list_create();
	break;
	case ACT_SET:
		recv(SOCKET_COORDINADOR,&paqueteSet,header.mSize,0);
		set(paqueteSet);

	break;
	case ACT_STORE:
		recv(SOCKET_COORDINADOR,&paqueteStore,header.mSize,0);
		store(paqueteStore);
	break;
	default:
		//procesa error
	break;
	}
	return true;
}

void respuesta(){
	//prepara la respueta que se le va a dar al coordinador

}

void compactar(){
	//realiza la compactacion de los datos en la memoria

}

void dumping(){
	//baja a disco todos los datos que fueron seteados en la memoria hasta el momento

}

void finCompactacion(){
	//cambia el estado de Compactando a Disponible

}

void finDumping(){
	//cambia el estado de Dumping a Disponible

}

int main(int argc, char* argv[]) {

	//creo el logger
	LOG_INSTANCIA = log_create("../log/logDeInstancia.log", "Instancia", true, LOG_LEVEL_TRACE);
	//se usa para escribir en el archivo de log y lo muestra por pantalla
	log_trace(LOG_INSTANCIA, "Iniciando Instancia");
	cargarConfig(argv[1]);

	int estado;
	/* 0: No Conectada
	 * 1: Conectada, configurandose
	 * 2: Disponible
	 * 3: Procesando Sentencia
	 * 4: Compactando
	 * 5: Dumping
	 * */
	estado = 0;

	while(1){

		switch(estado){
		case 0:
			puts("No conectada, se presenta al coordinador");
			if (conectarCoordinador()){
				estado = 1;
			}
		break;

		case 1:
			puts("Disponible, esperando que el coordinador envie una sentencia");
			if (recibirDeCoordinador()){
				estado = 2;
			}
		break;
			puts("Compactando, se compactaran los datos");
		break;

		case 5:
			puts("Dumping, dup de datos a disco");
		break;
		}
		sleep(3);
	}
	log_destroy(LOG_INSTANCIA);
	return EXIT_SUCCESS;
}
