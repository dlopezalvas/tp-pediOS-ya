#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void sindicato_process_request(int cod_op, int socket_client){
	void* mensaje;
	int size;

	switch(cod_op){
		case CONSULTAR_PLATOS:
			break;
		case GUARDAR_PEDIDO:
			break;
		case GUARDAR_PLATO:
			break;
		case CONFIRMAR_PEDIDO:
			break;
		case OBTENER_PEDIDO:
			break;
		case OBTENER_RESTAURANTE:
			break;
		case PLATO_LISTO:
			break;
		case OBTENER_RECETA:
			mensaje = recibir_mensaje(socket_client, &size);
			log_info(sindicatoLog, "me llego algo!!!");
			break;
		case TERMINAR_PEDIDO:
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}
}

void sindicato_serve_client(int socket){
	int cod_op;

	if(recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL) == 0){
		cod_op = -1;
	}

	sindicato_process_request(cod_op, socket);
}

void sindicato_wait_client(int server){
	pthread_t hilo;

	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (server, (void*) &direccion_cliente, &tam_direccion);

	pthread_create(&hilo,NULL,(void*)sindicato_serve_client,(void*)cliente);
	pthread_detach(hilo);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_server_initialize(){

	int sindicatoServer = iniciar_servidor(config_get_int_value(sindicatoConfig,"PUERTO_ESCUCHA"));
	if(sindicatoServer == -1){
		log_info(sindicatoLog, "No se pudo crear el servidor");
	} else{
		log_info(sindicatoLog, "Servidor Creado");
	}

	while(true) sindicato_wait_client(sindicatoServer);
}
