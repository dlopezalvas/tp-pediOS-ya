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

int main() {
	config_cliente = leer_config(PATH);
	log_cliente = iniciar_logger(config_cliente);

	pthread_mutex_init(&iniciar_consola_mtx, NULL);
	pthread_mutex_lock(&iniciar_consola_mtx);

	configurar_ip_puerto();

	mensajes_hilos = list_create();

	pthread_t consola;
	pthread_create(&consola, NULL, (void*)iniciar_consola, NULL);


	pthread_t conexion;
	pthread_create(&conexion, NULL, (void*)conexionRecepcion, NULL);
	pthread_detach(conexion);

	pthread_join(consola, NULL);


	return EXIT_SUCCESS;
}
