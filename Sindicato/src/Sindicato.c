#include "Sindicato.h"

int main(void) {

	sindicato_initialize();

	return EXIT_SUCCESS;
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_initialize(){
	sindicatoConfig = leer_config(SINDICATO_PATH_CONFIG);
	sindicatoLog = iniciar_logger(sindicatoConfig);

	//TODO: Inicializar el FS con punto de montaje
	sindicato_afip_initialize();

	sindicato_server_initialize();

	sindicato_console_initialize();

	config_destroy(sindicatoConfig);
	log_destroy(sindicatoLog);
}


