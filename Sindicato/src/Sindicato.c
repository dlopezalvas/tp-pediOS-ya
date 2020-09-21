#include "Sindicato.h"

int main(void) {

	t_config* sindicatoConfig = leer_config(PATH_CONFIG);

	t_log* sindicatoLog = iniciar_logger(sindicatoConfig);

	log_info(sindicatoLog, "chau"); //Delete this

	int sindicatoServer = iniciar_servidor(config_get_int_value(sindicatoConfig,"PUERTO_ESCUCHA"));

	if(sindicatoServer == -1){
		log_info(sindicatoLog, "No se pudo crear el servidor");
	} else{
		log_info(sindicatoLog, "Servidor Creado");
	}

	config_destroy(sindicatoConfig); //
	log_destroy(sindicatoLog);

	while(true)
		sindicato_wait_client(sindicatoServer);

	sindicato_initialize();

	return EXIT_SUCCESS;
}
