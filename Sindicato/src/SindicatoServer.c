#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_process_request(int cod_op, int socket_client){
	void* message = NULL;
	int size = 0;
	int sendMessage = NO_ENVIAR_RESPUESTA;
	uint32_t id_proceso;

	/* Response variables */
	int operationResult = 0;
	rta_obtenerRestaurante* restaurante;
	rta_obtenerReceta* receta;
	rta_obtenerPedido* pedido;
	t_restaurante_y_plato* platos;

	t_responseMessage* responseMessage = malloc(sizeof(t_responseMessage));
	responseMessage->message = malloc(sizeof(t_mensaje));
	//responseMessage->message->id=4;
	responseMessage->socket = socket_client;

	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		recv(socket_client, &id_proceso, sizeof(uint32_t), MSG_WAITALL);
		void* buffer = recibir_mensaje(socket_client, &size);
		message = deserializar_mensaje(buffer, cod_op);
		loggear_mensaje_recibido(message,cod_op, sindicatoLog);
	}

	switch(cod_op){
		case CONSULTAR_PLATOS:
			platos = sindicato_api_consultar_platos(message);

			responseMessage->message->tipo_mensaje = RTA_CONSULTAR_PLATOS;
			responseMessage->message->parametros = platos;

			sendMessage = ENVIAR_RESPUESTA;
			break;
		case GUARDAR_PEDIDO:
			operationResult = sindicato_api_guardar_pedido(message);

			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case GUARDAR_PLATO:
			operationResult = sindicato_api_guardar_plato(message);

			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PLATO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case CONFIRMAR_PEDIDO:
			operationResult = sindicato_api_confirmar_pedido(message);

			responseMessage->message->tipo_mensaje = RTA_CONFIRMAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case OBTENER_PEDIDO:
			pedido = sindicato_api_obtener_pedido(message);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_PEDIDO;
			responseMessage->message->parametros = pedido;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case OBTENER_RESTAURANTE:
			restaurante = sindicato_api_obtener_restaurante(message);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_RESTAURANTE;
			responseMessage->message->parametros = restaurante;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case PLATO_LISTO:
			operationResult = sindicato_api_plato_listo(message);

			responseMessage->message->tipo_mensaje = RTA_PLATO_LISTO;
			responseMessage->message->parametros = operationResult;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case OBTENER_RECETA:
			receta = sindicato_api_obtener_receta(message);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_RECETA;
			responseMessage->message->parametros = receta;
			sendMessage = ENVIAR_RESPUESTA;
			break;
		case TERMINAR_PEDIDO:
			operationResult = sindicato_api_terminar_pedido(message);

			responseMessage->message->tipo_mensaje = RTA_TERMINAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessage = ENVIAR_RESPUESTA;
			break;
	}

	if(sendMessage == ENVIAR_RESPUESTA)
		sindicato_api_send_response_of_operation(responseMessage);
}

void internal_serve_client(int socket){
	int cod_op;

	if(recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL) == -1){
		cod_op = -1;
	}

	if(cod_op != -1 && cod_op != POSICION_CLIENTE){
		internal_process_request(cod_op, socket);
	} else{
		if(cod_op == POSICION_CLIENTE){
			//TODO:Sincronizar log
			log_info(sindicatoDebugLog, "Handshake Cliente");
		}
		pthread_exit(NULL);
	}
}

void internal_wait_client(int server){
	pthread_t hilo;

	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept(server, (void*) &direccion_cliente, &tam_direccion);

	//TODO:Sincronizar log
	log_info(sindicatoLog, "Conexion aceptada");

	pthread_create(&hilo,NULL,(void*)internal_serve_client,(void*)cliente);
	pthread_detach(hilo);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_server_initialize(){

	int sindicatoServer = iniciar_servidor(config_get_int_value(sindicatoConfig,"PUERTO_ESCUCHA"));
	if(sindicatoServer == -1){
		log_info(sindicatoDebugLog, "No se pudo crear el servidor");
	} else{
		log_info(sindicatoDebugLog, "Servidor Creado");
	}

	while(true) internal_wait_client(sindicatoServer);
}
