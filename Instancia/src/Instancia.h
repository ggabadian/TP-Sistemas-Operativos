#ifndef INSTANCIA_H_
#define INSTANCIA_H_

#include "configInstancia.h"
#include "../../libs/socketClient.h"
#include "../../libs/protocolo.h"

int PUNTERO_CIRCULAR;
int CONTROL_LRU;
int SOCKET_COORDINADOR;
int CANTIDAD_ENTRADAS;
int TAMANIO_ENTRADAS;
char** ALMACENAMIENTO;
pthread_mutex_t MUTEX;
t_list* TABLA_ENTRADAS;
t_bitarray BIT_ARRAY;

t_log* LOG_INSTANCIA;

t_set paqueteSet;
t_store paqueteStore;
t_initInstancia paqueteInit;

typedef struct {
	char clave[40];
	int posicion;
	int tamanio;
	int cantidadUtilizada;
	int controlLRU;
}t_entrada;


int CONNECTED;


void enviarACoordinador(char* msg);
void conectarCoordinador();
void recibirOperacion();
void realizarSet();
bool hayEspacioContiguo(int tamanio, int* posicion);
bool hayEntradasDisponibles(int tamanio);
int entradasNecesarias(char* valor);
bool realizarStore(char clave[40]);
void correrAlgoritmoDeReemplazo();
bool yaExisteClave(char clave[40]);
void test_recibirSet(char* clave, char* valor);
void* atenderDump();
void realizarDump();
void enviarOrdenDeCompactar();
void compactar();
void realizarSet_Agregar(int entradasNecesarias);
void realizarSet_Actualizar(int entradasNecesarias);
t_entrada* obtenerDato(char* clave);
void algoritmoCircular();
void algoritmoLRU();
void algoritmoBSU();
t_entrada* obtenerDato_posicion(int posicion);
char* obtenerValor(char* clave);
char* valorEnDisco(char* clave);
void reincorporar(char* claves);
void liberarAlmacenamiento();
void liberarTablaEntradas();
void aumentarLRU(char* clave);
void mostarAlmacenamiento();
int main(int argc, char* argv[]);
void aumentarCircular();


#endif
