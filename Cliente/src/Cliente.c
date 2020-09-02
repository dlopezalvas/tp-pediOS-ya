/*
 ============================================================================
 Name        : Cliente.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include "Cliente.h"

int main(void) {

	config_cliente = leer_config(PATH);
	log_cliente = iniciar_logger(config_cliente);

//	cargar_configuracion();

	iniciar_consola();

	pthread_t pruebaConexion;
	pthread_create(&pruebaConexion, NULL, (void*)iniciarConexion, NULL);
	pthread_join(pruebaConexion, NULL);

	return EXIT_SUCCESS;
}
