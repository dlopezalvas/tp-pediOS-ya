#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void sindicato_process_request(int cod_op, int socket_client){
	void* message;
	void* buffer;
	int size;

	/* Response variables*/
	int operationResult;
	rta_obtenerRestaurante restaurante;
	rta_obtenerReceta receta;
	rta_obtenerPedido pedido;
	t_restaurante_y_plato platos;

	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		buffer = recibir_mensaje(socket_client, &size);
		message = deserializar_mensaje(buffer, cod_op);
	}

	switch(cod_op){
		case CONSULTAR_PLATOS:
			platos = sindicato_api_consultar_platos(message);
			break;
		case GUARDAR_PEDIDO:
			operationResult = sindicato_api_guardar_pedido(message);
			break;
		case GUARDAR_PLATO:
			operationResult = sindicato_api_guardar_plato(message);
			break;
		case CONFIRMAR_PEDIDO:
			operationResult = sindicato_api_confirmar_pedido(message);
			break;
		case OBTENER_PEDIDO:
			pedido = sindicato_api_obtener_pedido(message);
			break;
		case OBTENER_RESTAURANTE:
			restaurante = sindicato_api_obtener_restaurante(message);
			break;
		case PLATO_LISTO:
			operationResult = sindicato_api_plato_listo(message);
			break;
		case OBTENER_RECETA:
			receta = sindicato_api_obtener_receta(message);
			break;
		case TERMINAR_PEDIDO:
			operationResult = sindicato_api_terminar_pedido(message);
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}

	sindicato_api_send_response_of_operation();
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
