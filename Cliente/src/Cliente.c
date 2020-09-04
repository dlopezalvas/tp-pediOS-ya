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

int main(int argc, char* argv[]) {
	config_cliente = leer_config(PATH);
	log_cliente = iniciar_logger(config_cliente);
	if(argc !=2 || !validar_proceso(argc, argv)){
		puts("Por favor ingrese alguno de los siguiente parametros al ejecutar: \nComanda\nApp\nRestaurante\nSindicato");
		return EXIT_SUCCESS;
	}

	pthread_mutex_init(&iniciar_consola_mtx, NULL);
	pthread_mutex_lock(&iniciar_consola_mtx);

	proceso = string_duplicate(argv[1]);
//	proceso = COMANDA;
	string_to_upper(proceso);
	configurar_ip_puerto();

	mensajes_a_enviar = queue_create();
	sem_init(&sem_mensajes_a_enviar, 0, 0);
	pthread_t consola;
	pthread_create(&consola, NULL, (void*)iniciar_consola, NULL);


	pthread_t envio_mensajes;
	pthread_create(&envio_mensajes, NULL, (void*)conexionEnvio, NULL);
	pthread_detach(envio_mensajes);


//	pthread_t conexion2;
//	pthread_create(&conexion2, NULL, (void*)conexionEnvio, NULL);
//	pthread_join(conexion2, NULL);

	pthread_join(consola, NULL);


	return EXIT_SUCCESS;
}
