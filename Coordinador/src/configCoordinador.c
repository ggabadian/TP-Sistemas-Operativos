#include "configCoordinador.h"

void cargarConfig() {
	t_config *configCoord = config_create(PATH_COORDINADOR_CONFIG);

	IP = strdup(config_get_string_value(configCoord, "IP"));
	PUERTO = strdup(config_get_string_value(configCoord, "PUERTO"));
	BACKLOG = config_get_int_value(configCoord,"BACKLOG");
	PACKAGESIZE = config_get_int_value(configCoord, "PACKAGESIZE");

	config_destroy(configCoord);
}
