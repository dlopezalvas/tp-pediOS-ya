#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_process_request(int cod_op, int socket_client){
	void* message;
	void* buffer;
	int size;

	/* Response variables*/
	int operationResult;
	rta_obtenerRestaurante restaurante;
	rta_obtenerReceta receta;
	rta_obtenerPedido pedido;
	t_restaurante_y_plato platos;

	t_responseMessage responseMessage = malloc(sizeof(t_responseMessage));
	responseMessage->message = malloc(sizeof(t_mensaje));
	responseMessage->socket = socket_client;

	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		buffer = recibir_mensaje(socket_client, &size);
		message = deserializar_mensaje(buffer, cod_op);
	}

	switch(cod_op){
		case CONSULTAR_PLATOS:
			log_info(sindicatoLog, "CONSULTAR_PLATOS");
			platos = sindicato_api_consultar_platos(message);
			responseMessage->message->tipo_mensaje = RTA_CONSULTAR_PLATOS;
			responseMessage->message->parametros = platos;
			break;
		case GUARDAR_PEDIDO:
			log_info(sindicatoLog, "GUARDAR_PEDIDO");
			operationResult = sindicato_api_guardar_pedido(message);
			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PEDIDO;
			responseMessage->message->parametros = operationResult;
			break;
		case GUARDAR_PLATO:
			log_info(sindicatoLog, "GUARDAR_PLATO");
			operationResult = sindicato_api_guardar_plato(message);
			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PLATO;
			responseMessage->message->parametros = operationResult;
			break;
		case CONFIRMAR_PEDIDO:
			log_info(sindicatoLog, "CONFIRMAR_PEDIDO");
			operationResult = sindicato_api_confirmar_pedido(message);
			responseMessage->message->tipo_mensaje = RTA_CONFIRMAR_PEDIDO;
			responseMessage->message->parametros = operationResult;
			break;
		case OBTENER_PEDIDO:
			log_info(sindicatoLog, "OBTENER_PEDIDO");
			pedido = sindicato_api_obtener_pedido(message);
			responseMessage->message->tipo_mensaje = RTA_OBTENER_PEDIDO;
			responseMessage->message->parametros = pedido;
			break;
		case OBTENER_RESTAURANTE:
			log_info(sindicatoLog, "OBTENER_RESTAURANTE");
			restaurante = sindicato_api_obtener_restaurante(message);
			responseMessage->message->tipo_mensaje = RTA_OBTENER_RESTAURANTE;
			responseMessage->message->parametros = restaurante;
			break;
		case PLATO_LISTO:
			log_info(sindicatoLog, "PLATO_LISTO");
			operationResult = sindicato_api_plato_listo(message);
			responseMessage->message->tipo_mensaje = RTA_PLATO_LISTO;
			responseMessage->message->parametros = operationResult;
			break;
		case OBTENER_RECETA:
			log_info(sindicatoLog, "OBTENER_RECETA");
			receta = sindicato_api_obtener_receta(message);
			responseMessage->message->tipo_mensaje = RTA_OBTENER_RECETA;
			responseMessage->message->parametros = receta;
			break;
		case TERMINAR_PEDIDO:
			log_info(sindicatoLog, "TERMINAR_PEDIDO");
			operationResult = sindicato_api_terminar_pedido(message);
			responseMessage->message->tipo_mensaje = RTA_TERMINAR_PEDIDO;
			responseMessage->message->parametros = operationResult;
			break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
	}

	sindicato_api_send_response_of_operation(responseMessage);
	//TODO: loguear respuesta
}

void internal_serve_client(int socket){
	int cod_op;

	if(recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL) == 0){
		cod_op = -1;
	}

	internal_process_request(cod_op, socket);
}

void internal_wait_client(int server){
	pthread_t hilo;

	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (server, (void*) &direccion_cliente, &tam_direccion);

	//TODO: Loguear: nueva conexion

	pthread_create(&hilo,NULL,(void*)internal_serve_client,(void*)cliente);
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

	while(true) internal_wait_client(sindicatoServer);
}
