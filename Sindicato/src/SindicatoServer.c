#include "SindicatoServer.h"

/* ********************************** PRIVATE FUNCTIONS ********************************** */

void internal_process_request(int cod_op, int socket_client){
	void* messageReceived = NULL;
	int sizeMessage = 0;
	int sendMessageFlag = NO_ENVIAR_RESPUESTA;
	uint32_t id_proceso;

	/* Response variables */
	uint32_t* operationResult = 0;
	rta_obtenerRestaurante* restaurante;
	rta_obtenerReceta* receta;
	rta_obtenerPedido* pedido;
	t_restaurante_y_plato* platos;

	t_responseMessage* responseMessage = malloc(sizeof(t_responseMessage));
	responseMessage->message = malloc(sizeof(t_mensaje));
	responseMessage->message->id = sindicatoProcessId;
	responseMessage->socket = socket_client;

	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		recv(socket_client, &id_proceso, sizeof(uint32_t), MSG_WAITALL);
		void* buffer = recibir_mensaje(socket_client, &sizeMessage);
		messageReceived = deserializar_mensaje(buffer, cod_op);
		loggear_mensaje_recibido(messageReceived,cod_op, sindicatoLog);
	}

	switch(cod_op){
		case CONSULTAR_PLATOS:
			platos = sindicato_api_consultar_platos(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_CONSULTAR_PLATOS;
			responseMessage->message->parametros = platos;

			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case GUARDAR_PEDIDO:
			operationResult = sindicato_api_guardar_pedido(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case GUARDAR_PLATO:
			operationResult = sindicato_api_guardar_plato(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_GUARDAR_PLATO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case CONFIRMAR_PEDIDO:
			operationResult = sindicato_api_confirmar_pedido(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_CONFIRMAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case OBTENER_PEDIDO:
			pedido = sindicato_api_obtener_pedido(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_PEDIDO;
			responseMessage->message->parametros = pedido;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case OBTENER_RESTAURANTE:
			restaurante = sindicato_api_obtener_restaurante(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_RESTAURANTE;
			responseMessage->message->parametros = restaurante;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case PLATO_LISTO:
			operationResult = sindicato_api_plato_listo(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_PLATO_LISTO;
			responseMessage->message->parametros = operationResult;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case OBTENER_RECETA:
			receta = sindicato_api_obtener_receta(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_OBTENER_RECETA;
			responseMessage->message->parametros = receta;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
		case TERMINAR_PEDIDO:
			operationResult = sindicato_api_terminar_pedido(messageReceived);

			responseMessage->message->tipo_mensaje = RTA_TERMINAR_PEDIDO;
			responseMessage->message->parametros = (void*)operationResult;
			sendMessageFlag = ENVIAR_RESPUESTA;
			break;
	}

	if(sendMessageFlag == ENVIAR_RESPUESTA)
		sindicato_api_send_response_of_operation(responseMessage);

	//TODO: Liberar memoraia de todos los mensajes.
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
			log_info(sindicatoDebugLog, "[SERVER] Handshake Cliente");

			///TODO: enviar mensaje que no voy a utilizar el socket de escucha
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
	log_info(sindicatoLog, "[SERVER] Conexion aceptada");

	pthread_create(&hilo,NULL,(void*)internal_serve_client,(void*)cliente);
	pthread_detach(hilo);
}

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_server_initialize(){

	int sindicatoServer = iniciar_servidor(sindicatoPort);
	if(sindicatoServer == -1){
		log_info(sindicatoDebugLog, "[SERVER] No se pudo crear el servidor");
	} else{
		log_info(sindicatoDebugLog, "[SERVER] Servidor Creado");
	}

	while(true) internal_wait_client(sindicatoServer);
}
