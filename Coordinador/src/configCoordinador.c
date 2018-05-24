#include "configCoordinador.h"

void cargarConfig() {
	t_config *configCoord = config_create(PATH_COORDINADOR_CONFIG);

	PUERTO = config_get_string_value(configCoord, "PUERTO");
	ALGORITMO = config_get_string_value(configCoord, "ALGORITMO");
	CANTIDAD_ENTRADAS = config_get_int_value(configCoord,"CANTIDAD_ENTRADAS");
	BYTES_ENTRADA = config_get_int_value(configCoord, "BYTES_ENTRADA");
	RETARDO = config_get_int_value(configCoord, "RETARDO");

	config_destroy(configCoord);
}
