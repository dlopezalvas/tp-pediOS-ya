#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

/* SERVER FUNCTIONS */

void sindicato_process_request(int cod_op, int socket_client){

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
		case 1: //OBTENER_RECETA not defined yet
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
		puts("error");
	}

	sindicato_process_request(cod_op, socket);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_wait_client(int server){
	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (server, (void*) &direccion_cliente, &tam_direccion);

	pthread_t hilo;

	//pthread_mutex_lock(&hilos_clientes_mtx);
	//list_add(hilos_clientes, &hilo);
	//pthread_mutex_unlock(&hilos_clientes_mtx);

	pthread_create(&hilo,NULL,(void*)sindicato_serve_client,(void*)cliente);
	pthread_detach(hilo);
}
