#include "Sindicato.h"

int main(void) {

	sindicato_initialize();

	return EXIT_SUCCESS;
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_initialize(){
	sindicatoConfig = leer_config(SINDICATO_PATH_CONFIG);

	/* logs folder creation */
	sindicato_utils_create_folder(config_get_string_value(sindicatoConfig, "LOGS_PATH"), true);

	sindicatoLog = iniciar_logger(sindicatoConfig);
	sindicatoDebugLog = sindicato_utils_iniciar_debug_logger(sindicatoConfig);

	/* Set global variables */
	sindicatoProcessId = (uint32_t)config_get_int_value(sindicatoConfig, "SINDICATO_ID");
	sindicatoPort = config_get_int_value(sindicatoConfig,"PUERTO_ESCUCHA"); //TODO: Se puede optimizar
	sindicatoMountPoint = config_get_string_value(sindicatoConfig,"PUNTO_MONTAJE");
	
	config_destroy(sindicatoConfig);

	sindicato_api_afip_initialize();

	/* Initialize the thread that listen conections */
	pthread_t sindicatoServerThread;
	pthread_create(&sindicatoServerThread, NULL, (void*)sindicato_server_initialize, NULL);
	pthread_detach(sindicatoServerThread);

	/* Initialize the thread that open the console */
	pthread_t sindicatoConsoleThread;
	pthread_create(&sindicatoConsoleThread, NULL, (void*)sindicato_console_initialize,NULL);
	pthread_join(sindicatoConsoleThread, NULL);

	log_destroy(sindicatoLog);
	log_destroy(sindicatoDebugLog);
}


