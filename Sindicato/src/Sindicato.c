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
	sindicato_api_afip_initialize();

	/* Initialize the thread that listen conections */
	pthread_t sindicatoServerThread;
	pthread_create(&sindicatoServerThread, NULL, (void*)sindicato_server_initialize, NULL);
	pthread_detach(sindicatoServerThread);

	/* Initialize the thread that open the console */
	pthread_t sindicatoConsoleThread;
	pthread_create(&sindicatoConsoleThread, NULL, (void*)sindicato_console_initialize,NULL);
	pthread_join(sindicatoConsoleThread, NULL);

	config_destroy(sindicatoConfig);
	log_destroy(sindicatoLog);
}


