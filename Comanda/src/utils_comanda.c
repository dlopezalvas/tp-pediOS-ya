#include "utils_comanda.h"

void iniciar_comanda(){

	config_comanda = leer_config(PATH);
	log_comanda = log_create("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/comanda.LOG","comanda",1,LOG_LEVEL_INFO);

	hilos_clientes = list_create();
	pthread_mutex_init(&hilos_clientes_mtx, NULL);

	hilos_operaciones = list_create();
	pthread_mutex_init(&hilos_operaciones_mtx, NULL);

	restaurantes = list_create();
	pthread_mutex_init(&restaurantes_mtx, NULL);

	memoria_principal = malloc(config_get_int_value(config_comanda, TAMANIO_MEMORIA));
	memoria_swap = malloc(config_get_int_value(config_comanda, TAMANIO_SWAP));

}

//void iniciar_colas(){
//
//	guardar_pedido_queue = queue_create();
//	pthread_mutex_init(&guardar_pedido_queue_mtx, NULL);
//	guardar_plato_queue = queue_create();
//	pthread_mutex_init(&guardar_plato_queue_mtx, NULL);
//	obtener_pedido_queue = queue_create();
//	pthread_mutex_init(&obtener_pedido_queue_mtx, NULL);
//	confimar_pedido_queue = queue_create();
//	pthread_mutex_init(&confimar_pedido_queue_mtx, NULL);
//	plato_listo_queue = queue_create();
//	pthread_mutex_init(&plato_listo_queue_mtx, NULL);
//	finalizar_pedido_queue = queue_create();
//	pthread_mutex_init(&finalizar_pedido_queue_mtx, NULL);
//
//}

void process_request(int cod_op, int cliente_fd){
	int size = 0;
	void* mensaje = NULL;
	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		void* buffer = recibir_mensaje(cliente_fd, &size);
		mensaje = deserializar_mensaje(buffer, cod_op);
	}

	loggear_mensaje_recibido(mensaje, cod_op, log_comanda);

	pthread_t hilo_operacion;
	t_mensaje_a_procesar* mensaje_a_procesar = malloc(sizeof(t_mensaje_a_procesar));
	mensaje_a_procesar->mensaje = mensaje;
	mensaje_a_procesar->socket_cliente = cliente_fd;

	pthread_mutex_lock(&hilos_operaciones_mtx);
	list_add(hilos_operaciones, &hilo_operacion);
	pthread_mutex_unlock(&hilos_operaciones_mtx);

	switch (cod_op) {
	case GUARDAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_guardar_pedido, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	case GUARDAR_PLATO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_guardar_plato, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	case CONFIRMAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_confirmar_pedido, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	case PLATO_LISTO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_plato_listo, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	case OBTENER_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_obtener_pedido, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	case FINALIZAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_finalizar_pedido, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	default:
		puts("error");
	}
}

void serve_client(int socket){

	while(1){
		int cod_op;
		if(recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL) == 0){
			cod_op = -1;
			//intento de reconexion
			puts("error");
		}else{
			process_request(cod_op, socket);
		}
	}
}

void esperar_cliente(int servidor){

	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (servidor, (void*) &direccion_cliente, &tam_direccion);

	pthread_t hilo;

	pthread_mutex_lock(&hilos_clientes_mtx);
	list_add(hilos_clientes, &hilo);
	pthread_mutex_unlock(&hilos_clientes_mtx);

	pthread_create(&hilo,NULL,(void*)serve_client,(void*)cliente);
	pthread_detach(hilo);

}

//ejecuciones

void ejecucion_guardar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;
	t_restaurante* restaurante = buscarRestaurante(mensaje->nombre.nombre);;
	uint32_t confirmacion;

	//fijarse si esta llena la memoria
	if(restaurante == NULL){
		restaurante = malloc(sizeof(t_restaurante));
		strcpy(restaurante->nombre, mensaje->nombre.nombre);
		pthread_mutex_init(&(restaurante->tabla_segmentos_mtx), NULL);
		restaurante->tabla_segmentos = list_create();
		pthread_mutex_lock(&restaurantes_mtx);
		list_add(restaurantes, restaurante);
		pthread_mutex_unlock(&restaurantes_mtx);
	}
	t_segmento* pedido = malloc(sizeof(t_segmento));
	pedido->id_pedido = mensaje->id;
	pedido->tabla_paginas = list_create();

	pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
	list_add(restaurante->tabla_segmentos, pedido);
	pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

	confirmacion = 1;

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_GUARDAR_PEDIDO);

}

void ejecucion_guardar_plato(t_mensaje_a_procesar* mensaje_a_procesar){
	m_guardarPlato* mensaje = mensaje_a_procesar->mensaje;
	uint32_t confirmacion;
	t_segmento* pedido = buscarPedido(mensaje->idPedido, mensaje->restaurante.nombre);
	t_pagina* plato = NULL;
	t_plato* plato_a_guardar = malloc(sizeof(t_plato));
	plato_a_guardar->cant_pedida = mensaje->cantidad;
	plato_a_guardar->cant_lista = 0;
	strcpy(plato_a_guardar->nombre, mensaje->restaurante.nombre);

	if(pedido != NULL){
		plato = buscarPlato(pedido->tabla_paginas, mensaje->comida.nombre);
		if(plato == NULL){
			//ver si hay memoria
			plato = malloc(sizeof(t_pagina));
			plato->modificado = false;
			plato->uso = true;
			plato->ultimo_acceso = time(NULL);
			//buscar frame
		}else{
			//buscar y traer de swap
			//deserializar
			//sumar plato deserializado a plato_a_guardar
		}
		//serializar
		//guardar en memoria
		confirmacion = 1;
	}else{
		//no existe pedido o restaurante
		confirmacion = 0;
	}

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_GUARDAR_PLATO);
}



void ejecucion_finalizar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;
}

void ejecucion_confirmar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	uint32_t * mensaje = mensaje_a_procesar->mensaje;
}

void ejecucion_plato_listo(t_mensaje_a_procesar* mensaje_a_procesar){
	m_platoListo* mensaje = mensaje_a_procesar->mensaje;
}

void ejecucion_obtener_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;
}


t_restaurante* buscarRestaurante(char* nombre){
	bool _mismoRestaurante(t_restaurante* restaurante){
		return string_equals_ignore_case(restaurante->nombre, nombre);
	}
	pthread_mutex_lock(&restaurantes_mtx);
	t_restaurante* restaurante = list_find(restaurantes, (void*)_mismoRestaurante);
	pthread_mutex_unlock(&restaurantes_mtx);
	return restaurante;
}

t_segmento* buscarPedido(uint32_t id_pedido, char* nombre){
	t_restaurante* restaurante = buscarRestaurante(nombre);
	t_segmento* pedido;

	bool _mismoPedido(t_segmento* segmento){
		return id_pedido == segmento->id_pedido;
	}

	if(restaurante != NULL){
	pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
	pedido = list_find(restaurante->tabla_segmentos, (void*)_mismoPedido);
	pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);
	}else{
		pedido = NULL;
	}
	return pedido;
}


void enviar_confirmacion(uint32_t _confirmacion, int cliente, op_code cod_op){
	t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));
	mensaje_a_enviar->tipo_mensaje = cod_op;
	uint32_t* confirmacion = malloc(sizeof(uint32_t));
	*confirmacion = _confirmacion;
	mensaje_a_enviar->parametros = confirmacion;
	enviar_mensaje(mensaje_a_enviar, cliente);
}


void* serializar_pagina(t_plato* plato){
	void* stream = malloc(TAMANIO_PAGINA);
	int offset = 0;

	memcpy(stream + offset, &plato->cant_lista, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &plato->cant_pedida, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, plato->nombre, TAMANIO_NOMBRE);

	return stream;
}

t_plato* deserializar_pagina(void* stream){
	int offset = 0;
	t_plato* plato = malloc(sizeof(t_plato));
	memcpy(&plato->cant_lista, stream + offset, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(&plato->cant_pedida, stream + offset , sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(plato->nombre, stream + offset, TAMANIO_NOMBRE);

	return stream;
}

t_pagina* buscarPlato(t_list* tabla_paginas, char* comida){
	return NULL;

}

