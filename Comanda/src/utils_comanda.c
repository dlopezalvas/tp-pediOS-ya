#include "utils_comanda.h"

void iniciar_comanda(){

	config_comanda = leer_config(PATH);
	log_comanda = log_create("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/comanda.LOG","comanda",1,LOG_LEVEL_INFO);

	if(string_equals_ignore_case(config_get_string_value(config_comanda, ALGORITMO_REEMPLAZO), "LRU")) algoritmo_reemplazo = LRU;
	else if(string_equals_ignore_case(config_get_string_value(config_comanda, ALGORITMO_REEMPLAZO), "CLOCK_MEJ")) algoritmo_reemplazo = CLOCK_MEJORADO;

	hilos_clientes = list_create();
	pthread_mutex_init(&hilos_clientes_mtx, NULL);

	hilos_operaciones = list_create();
	pthread_mutex_init(&hilos_operaciones_mtx, NULL);

	restaurantes = list_create();
	pthread_mutex_init(&restaurantes_mtx, NULL);

	paginas_swap = list_create();
	pthread_mutex_init(&paginas_swap_mtx, NULL);

	memoria_principal = malloc(config_get_int_value(config_comanda, TAMANIO_MEMORIA));
	pthread_mutex_init(&memoria_principal_mtx, NULL);

	memoria_swap = malloc(config_get_int_value(config_comanda, TAMANIO_SWAP));
	pthread_mutex_init(&memoria_swap_mtx, NULL);

	cant_frames_swap = (config_get_int_value(config_comanda, TAMANIO_SWAP) / TAMANIO_PAGINA) * sizeof(uint32_t);
	cant_frames_MP = (config_get_int_value(config_comanda, TAMANIO_MEMORIA) / TAMANIO_PAGINA) * sizeof(uint32_t);

	frames_swap = malloc(cant_frames_swap*sizeof(uint32_t));
	pthread_mutex_init(&frames_swap_mtx, NULL);

	frames_MP = malloc(cant_frames_MP*sizeof(uint32_t));
	pthread_mutex_init(&frames_MP_mtx, NULL);

	for(int i=0; i<cant_frames_swap; i++){
		frames_swap[i]=0;
	}
	for(int j=0; j<cant_frames_MP; j++){
		frames_MP[j]=0;
	}

}

void serve_client(int socket){

	while(1){
		int cod_op;
		int _recv = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if(_recv == 0 || _recv == -1){
			cod_op = -1;
			//intento de reconexion
			puts("error");
			liberar_conexion(socket);
			pthread_exit(NULL);
		}else{
			process_request(cod_op, socket);
		}
	}
}

void process_request(int cod_op, int cliente_fd){
	int size = 0;
	void* mensaje;
	uint32_t id_cliente;

	int _recv = recv(cliente_fd, &id_cliente, sizeof(uint32_t), MSG_WAITALL);

	if(_recv == 0 || _recv == -1){
		//intento de reconexion
		puts("error adentro process request");
		liberar_conexion(cliente_fd);
		pthread_exit(NULL);
	}

	void* buffer = recibir_mensaje(cliente_fd, &size);
	mensaje = deserializar_mensaje(buffer, cod_op);


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
	case POSICION_CLIENTE:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_handshake_cliente, mensaje_a_procesar);
		pthread_detach(hilo_operacion);
		break;
	default:
		puts("erro en el switch??");
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
	int frame_disponible_swap;
	m_guardarPlato* mensaje = mensaje_a_procesar->mensaje;
	uint32_t confirmacion = 0;
	t_restaurante* restaurante = buscarRestaurante(mensaje->restaurante.nombre);
	t_segmento* pedido;
	t_pagina* plato = NULL;

	if(restaurante != NULL){
		pedido = buscarPedido(mensaje->idPedido, restaurante);

		if(pedido != NULL){
			pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
			plato = buscarPlato(pedido->tabla_paginas, mensaje->comida.nombre);
			pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

			if(plato == NULL){
				frame_disponible_swap = memoria_disponible_swap();

				if(frame_disponible_swap != -1){

					t_plato* plato_a_guardar = malloc(sizeof(t_plato));
					plato_a_guardar->cant_pedida = mensaje->cantidad;
					plato_a_guardar->cant_lista = 0;
					strcpy(plato_a_guardar->nombre, mensaje->restaurante.nombre);

					plato = malloc(sizeof(t_pagina));
					plato->modificado = false;
					plato->uso = true;
					plato->ultimo_acceso = time(NULL);
					plato->pagina_swap = frame_disponible_swap;

					pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
					list_add(pedido->tabla_paginas, plato);
					pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

					pthread_mutex_lock(&paginas_swap_mtx);
					list_add(paginas_swap, plato);
					pthread_mutex_unlock(&paginas_swap_mtx);

					guardar_en_swap(frame_disponible_swap, plato_a_guardar);
					guardar_en_mp(plato_a_guardar);
				}
			}else{

				if(!plato->presencia){
					//TODO traer de swap
				}

				pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
				actualizar_plato_mp(plato, mensaje->cantidad);
				pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);
			}
			confirmacion = 1;
		}
	}

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_GUARDAR_PLATO);
}

void guardar_en_mp(t_plato* plato){
	int frame = seleccionar_frame_mp();
	void* pagina_serializada = serializar_pagina(plato);

	int offset = frame * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_principal_mtx);
	memcpy(memoria_principal + offset, pagina_serializada, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_principal_mtx);

}

int seleccionar_frame_mp(){

	int frame_disponible = memoria_disponible_mp();

	if(frame_disponible == -1){
		switch(algoritmo_reemplazo){
		case LRU:
			//			frame_disponible = eleccion_victima_LRU();
			break;
		case CLOCK_MEJORADO:
			//			frame_disponible = eleccion_victima_clock_mejorado();
			break;
		}
	}

	return frame_disponible;
}



void actualizar_plato_mp(t_pagina* pagina, int cantidad_pedida){

	void* plato_a_deserializar = malloc(TAMANIO_PAGINA);
	void* plato_serializado;

	t_plato* plato;

	int offset = pagina->frame * TAMANIO_PAGINA;

	pagina->modificado = true;
	pagina->ultimo_acceso = time(NULL);
	pagina->uso = true;

	pthread_mutex_lock(&memoria_principal_mtx);
	memcpy(plato_a_deserializar, memoria_principal + offset, TAMANIO_PAGINA);

	plato = deserializar_pagina(plato_a_deserializar);

	plato->cant_pedida += cantidad_pedida;

	//guardo en la mp
	plato_serializado = serializar_pagina(plato);

	memcpy(memoria_principal + offset, plato_serializado, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_principal_mtx);

	free(plato_a_deserializar);
	free(plato);
	free(plato_serializado);

}

void guardar_en_swap(int frame_destino_swap, t_plato* plato){
	void* pagina = serializar_pagina(plato);

	int offset = frame_destino_swap * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_swap_mtx);
	memcpy(memoria_swap + offset, pagina, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_swap_mtx);
}

int memoria_disponible_swap(){

	int frame_disponible = -1;
	int i = 0;

	pthread_mutex_lock(&frames_swap_mtx);
	while(frames_swap[i] != 0 && i < cant_frames_swap){
		i++;
	}

	if(frames_swap[i] == 0){
		frame_disponible = i;
		frames_swap[i] = 1;
	}
	pthread_mutex_unlock(&frames_swap_mtx);

	return frame_disponible;
}

int memoria_disponible_mp(){

	int frame_disponible = -1;
	int i = 0;

	pthread_mutex_lock(&frames_MP_mtx);
	while(frames_MP[i] != 0 && i < cant_frames_MP){
		i++;
	}

	if(frames_MP[i] == 0){
		frame_disponible = i;
		frames_MP[i] = 1;
	}
	pthread_mutex_unlock(&frames_MP_mtx);

	return frame_disponible;
}

void guardar_pagina(t_plato* plato){
	void* pagina = serializar_pagina(plato);

}

void ejecucion_finalizar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;
	t_restaurante* restaurante = buscarRestaurante(mensaje->nombre.nombre);
	t_segmento* pedido;
	uint32_t confirmacion = 0;

	bool es_pedido(t_segmento* _pedido){
		return _pedido->id_pedido == pedido->id_pedido;
	}

	if(restaurante != NULL){
		pedido = buscarPedido(mensaje->id, restaurante);
		if(pedido != NULL){
			pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
			pedido = list_remove_by_condition(restaurante->tabla_segmentos, (void*) es_pedido);
			pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

			list_iterate(pedido->tabla_paginas, (void*)liberar_pagina);
			list_destroy_and_destroy_elements(pedido->tabla_paginas, (void*)free_pagina);
			confirmacion = 1;
			free(pedido);
		}
	}
	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_FINALIZAR_PEDIDO);
}

void liberar_pagina(t_pagina* pagina){
	pthread_mutex_lock(&frames_MP_mtx);
	frames_MP[pagina->frame] = 0;
	pthread_mutex_unlock(&frames_MP_mtx);

	pthread_mutex_lock(&frames_swap_mtx);
	frames_swap[pagina->pagina_swap] = 0;
	pthread_mutex_unlock(&frames_swap_mtx);
}

void free_pagina(t_pagina* pagina){ //TODO fijarnos si list_destroy elimina los elementos o no :o
	free(pagina);
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

void ejecucion_handshake_cliente(t_mensaje_a_procesar* mensaje_a_procesar){
	t_coordenadas* mensaje = mensaje_a_procesar->mensaje;
	enviar_confirmacion(0, mensaje_a_procesar->socket_cliente, RTA_POSICION_CLIENTE);
	free_struct_mensaje(mensaje, POSICION_CLIENTE);
	free(mensaje_a_procesar);
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

t_segmento* buscarPedido(uint32_t id_pedido, t_restaurante* restaurante){
	t_segmento* pedido;

	bool _mismoPedido(t_segmento* segmento){
		return id_pedido == segmento->id_pedido;
	}
	pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
	pedido = list_find(restaurante->tabla_segmentos, (void*)_mismoPedido);
	pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

	return pedido;
}

void enviar_confirmacion(uint32_t _confirmacion, int cliente, op_code cod_op){
	t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));
	mensaje_a_enviar->tipo_mensaje = cod_op;
	mensaje_a_enviar->id = config_get_int_value(config_comanda, ID_COMANDA);
	uint32_t* confirmacion = malloc(sizeof(uint32_t));
	*confirmacion = _confirmacion;
	mensaje_a_enviar->parametros = confirmacion;
	enviar_mensaje(mensaje_a_enviar, cliente);
	loggear_mensaje_enviado(confirmacion, cod_op, log_comanda);
	free_struct_mensaje(confirmacion,cod_op);
	free(mensaje_a_enviar);
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

	t_plato* plato;
	t_pagina* pagina;
	void* stream;

	int offset;

	for(int i = 0; i < list_size(tabla_paginas); i++){
		stream = malloc(TAMANIO_PAGINA);
		pagina = list_get(tabla_paginas, i);
		offset = pagina->pagina_swap * TAMANIO_PAGINA;
		pthread_mutex_lock(&memoria_swap_mtx);
		memcpy(stream, memoria_swap + offset, TAMANIO_PAGINA);
		pthread_mutex_unlock(&memoria_swap_mtx);
		plato = deserializar_pagina(stream);
		free(stream);
		if(string_equals_ignore_case(plato->nombre, comida)){
			free(plato); //TODO ver que no rompa
			return pagina;
		}
		free(plato);
	}

	return NULL;

}

