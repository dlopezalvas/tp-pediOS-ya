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

	inicializar_swap();

	cant_frames_swap = (config_get_int_value(config_comanda, TAMANIO_SWAP) / TAMANIO_PAGINA);
	cant_frames_MP = (config_get_int_value(config_comanda, TAMANIO_MEMORIA) / TAMANIO_PAGINA);

	int bytes_swap = ceil((float)cant_frames_swap / 8);
	int bytes_mp = ceil((float)cant_frames_swap / 8);

	char* mem_frames_swap = malloc(bytes_swap);
	frames_swap = bitarray_create(mem_frames_swap, bytes_swap);

	pthread_mutex_init(&frames_swap_mtx, NULL);

	char* mem_frames_MP = malloc(bytes_mp);

	frames_MP = bitarray_create(mem_frames_MP, bytes_mp);
	pthread_mutex_init(&frames_MP_mtx, NULL);

	for(int i=0; i<cant_frames_swap; i++){

		bitarray_clean_bit(frames_swap, i);
	}

	for(int j=0; j<cant_frames_MP; j++){
		bitarray_clean_bit(frames_MP, j);
	}

	puntero_clock = 0;
	pthread_mutex_init(&puntero_clock_mtx, NULL);

}

void inicializar_swap(){

	pthread_mutex_init(&memoria_swap_mtx, NULL);

	int tamanio_swap = config_get_int_value(config_comanda, TAMANIO_SWAP);

	int archivo_swap = open("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/Swap.bin", O_RDWR | O_CREAT, 0700);  //uso open porque necesito el int para el mmap

	ftruncate(archivo_swap, tamanio_swap);

	memoria_swap = mmap(NULL, tamanio_swap, PROT_WRITE | PROT_READ, MAP_SHARED, archivo_swap, 0);

	close(archivo_swap);

}

void serve_client(int socket){

	while(1){
		int cod_op;
		int _recv = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if(_recv == 0 || _recv == -1){
			cod_op = -1;
			//intento de reconexion
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

	free(buffer);
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

void ejecucion_guardar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){ //listo ponele
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;
	t_restaurante* restaurante = buscarRestaurante(mensaje->nombre.nombre);;
	uint32_t confirmacion;
	if(restaurante == NULL){
		restaurante = malloc(sizeof(t_restaurante));
		restaurante->nombre = malloc(mensaje->nombre.largo_nombre +1);
		strcpy(restaurante->nombre, mensaje->nombre.nombre);
		pthread_mutex_init(&(restaurante->tabla_segmentos_mtx), NULL);
		restaurante->tabla_segmentos = list_create();
		pthread_mutex_lock(&restaurantes_mtx);
		list_add(restaurantes, restaurante);
		pthread_mutex_unlock(&restaurantes_mtx);
	}

	t_segmento* pedido = malloc(sizeof(t_segmento));
	pedido->id_pedido = mensaje->id;
	pedido->estado = PENDIENTE;
	pedido->tabla_paginas = list_create();

	pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
	list_add(restaurante->tabla_segmentos, pedido);
	pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

	confirmacion = 1;

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_GUARDAR_PEDIDO);
	free_struct_mensaje(mensaje, GUARDAR_PEDIDO);
	free(mensaje_a_procesar);

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
			if(pedido->estado != CONFIRMADO){
				pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
				plato = buscarPlato(pedido->tabla_paginas, mensaje->comida.nombre);
				pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

				if(plato == NULL){
					frame_disponible_swap = memoria_disponible_swap();

					if(frame_disponible_swap != -1){

						t_plato* plato_a_guardar = malloc(sizeof(t_plato));
						plato_a_guardar->cant_pedida = mensaje->cantidad;
						plato_a_guardar->cant_lista = 0;
						strncpy(plato_a_guardar->nombre, mensaje->comida.nombre, mensaje->comida.largo_nombre +1);//TODO ver si guardar '\0'

						plato = malloc(sizeof(t_pagina));
						plato->modificado = false;
						plato->uso = true;
						plato->ultimo_acceso = time(NULL);
						plato->pagina_swap = frame_disponible_swap;
						plato->presencia = true; //TODO ver si necesita mutex
						//TODO no se si va aca
						guardar_en_swap(frame_disponible_swap, plato_a_guardar);

						plato->frame = guardar_en_mp(plato_a_guardar);


						pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
						list_add(pedido->tabla_paginas, plato);
						pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

						pthread_mutex_lock(&paginas_swap_mtx);
						list_add(paginas_swap, plato);
						pthread_mutex_unlock(&paginas_swap_mtx);

						free(plato_a_guardar);
						confirmacion = 1;
					}
				}else{
					if(!plato->presencia){
						traer_de_swap(plato);
					}

					pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
					actualizar_plato_mp(plato, mensaje->cantidad, 0);
					pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);
					confirmacion = 1;
				}

			}
		}
	}

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_GUARDAR_PLATO);
	free_struct_mensaje(mensaje, GUARDAR_PLATO);
	free(mensaje_a_procesar);
}

int guardar_en_mp(t_plato* plato){
	int frame = seleccionar_frame_mp();
	void* pagina_serializada = serializar_pagina(plato);
	int offset = frame * TAMANIO_PAGINA;

//	printf("offset %d, tamanio pag %d, mem + offset %d", offset, sizeof(pagina_serializada), memoria_principal + offset);

	pthread_mutex_lock(&memoria_principal_mtx);
	memcpy(memoria_principal + offset, pagina_serializada, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_principal_mtx);
	free(pagina_serializada);
	return frame;
}

int seleccionar_frame_mp(){

	int frame_disponible = memoria_disponible_mp();

	if(frame_disponible == -1){
		switch(algoritmo_reemplazo){
		case LRU:
			puts("entro a lru");
			frame_disponible = eleccion_victima_LRU();
			break;
		case CLOCK_MEJORADO:
			frame_disponible = eleccion_victima_clock_mejorado();
			break;
		}
	}

	return frame_disponible;
}

int eleccion_victima_clock_mejorado(){

	t_pagina* victima = NULL;

	bool ordenar_por_frame(t_pagina* pagina1, t_pagina* pagina2){
		return pagina1->frame < pagina2->frame;
	}

	pthread_mutex_lock(&puntero_clock_mtx);

	pthread_mutex_lock(&paginas_swap_mtx);

	t_list* en_mp = list_filter(paginas_swap, (void*)esta_en_MP);

	pthread_mutex_unlock(&paginas_swap_mtx); //TODO ver

	list_sort(en_mp, (void*) ordenar_por_frame);

	while(victima == NULL){
		victima = list_iterate_and_find_from_index(en_mp, (void*)hacer_nada, (void*)uso_modificado_cero);
		if(victima == NULL){
			victima = list_iterate_and_find_from_index(en_mp, (void*)cambiar_uso_cero, (void*)uso_cero_modificado_uno);
		}
	}

	pthread_mutex_unlock(&puntero_clock_mtx);

	liberar_frame(victima);
	list_destroy(en_mp);

	return victima->frame;
}

void hacer_nada(void* algo){

}

void cambiar_uso_cero(t_pagina* pagina){
	pagina->uso = false;
}

bool uso_cero_modificado_uno(t_pagina* pagina){
	return !pagina->uso && pagina->modificado;
}

bool uso_modificado_cero(t_pagina* pagina){
	return (!pagina->uso && !pagina->modificado);
}

void* list_iterate_and_find_from_index(t_list* self, void(closure)(void*), bool(*condition)(void*)){
	t_pagina * pagina = list_get(self, puntero_clock);
//	t_pagina *aux = NULL;

	for(int i = puntero_clock; !condition(pagina) && i < self->elements_count; i++) {
		closure(pagina);
		pagina = list_get(self, i);
	}

	if(!condition(pagina)){
//		pagina = list_get(self, 0);

		for(int i = 0; i < puntero_clock && !condition(pagina); i++){
			pagina = list_get(self, 0);
			closure(pagina);
		}
	}

	if(!condition(pagina)){
		return NULL;
	}

	puntero_clock = pagina->frame;

	return pagina;

}

bool esta_en_MP(t_pagina* pagina){
	return pagina->presencia;
}

int eleccion_victima_LRU(){

	bool lru(t_pagina* pagina1, t_pagina* pagina2){
		return pagina1->ultimo_acceso < pagina2->ultimo_acceso;
	}

	t_pagina* victima;

	pthread_mutex_lock(&paginas_swap_mtx);

	list_sort(paginas_swap, (void*)lru);

	victima = list_find(paginas_swap, (void*)esta_en_MP); //las ordeno por LRU y agarro la primera en la lista que este ocupada

	puts("victima frame");


	pthread_mutex_unlock(&paginas_swap_mtx);

	liberar_frame(victima);

	return victima->frame;

}

void liberar_frame(t_pagina* victima){

	victima->ultimo_acceso = time(NULL);

	if(victima->modificado){
		actualizar_swap(victima);
		victima->modificado = false;

	}
	victima->presencia = false;
	victima->uso = false;

}

void actualizar_swap(t_pagina* pagina){
	void* plato = malloc(TAMANIO_PAGINA);

	int offset = pagina->frame * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_principal_mtx);
	memcpy(plato, memoria_principal + offset, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_principal_mtx);

	offset = pagina->pagina_swap * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_swap_mtx);
	memcpy(memoria_swap + offset, plato, TAMANIO_PAGINA);
	msync(memoria_swap, sizeof(memoria_swap), MS_SYNC);
	pthread_mutex_unlock(&memoria_swap_mtx);

	free(plato);

}

void actualizar_plato_mp(t_pagina* pagina, int cantidad_pedida, int cantidad_lista){

	void* plato_a_deserializar = malloc(TAMANIO_PAGINA);
	void* plato_serializado;

	t_plato* plato;

	int offset = pagina->frame * TAMANIO_PAGINA;

	pagina->modificado = true;
	pagina->ultimo_acceso = time(NULL);
	pagina->uso = true;
	pagina->presencia = true;

	pthread_mutex_lock(&memoria_principal_mtx);
	memcpy(plato_a_deserializar, memoria_principal + offset, TAMANIO_PAGINA);

	plato = deserializar_pagina(plato_a_deserializar);

	plato->cant_pedida += cantidad_pedida;

	plato->cant_lista += cantidad_lista;
	if(plato->cant_pedida < plato->cant_lista){
		puts("error ya estan listos todos los pedidos");
		plato->cant_lista = plato->cant_pedida;
	}

	//guardo en la mp
	plato_serializado = serializar_pagina(plato);

	memcpy(memoria_principal + offset, plato_serializado, TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_principal_mtx);

	free(plato_a_deserializar);
	free(plato);
	free(plato_serializado);

}

void traer_de_swap(t_pagina* pagina){

	t_plato* plato;

	void* plato_a_deserializar = malloc(TAMANIO_PAGINA);

	int offset = pagina->pagina_swap * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_swap_mtx);
	memcpy(plato_a_deserializar, memoria_swap + offset,  TAMANIO_PAGINA);
	pthread_mutex_unlock(&memoria_swap_mtx);

	plato = deserializar_pagina(plato_a_deserializar);
	free(plato_a_deserializar);

	guardar_en_mp(plato);
	pagina->presencia = true;
	pagina->uso = true;
	pagina->ultimo_acceso = time(NULL);
	free(plato);

}

void guardar_en_swap(int frame_destino_swap, t_plato* plato){
	void* pagina = serializar_pagina(plato);

	int offset = frame_destino_swap * TAMANIO_PAGINA;

	pthread_mutex_lock(&memoria_swap_mtx);
	memcpy(memoria_swap + offset, pagina, TAMANIO_PAGINA);
	msync(memoria_swap, sizeof(memoria_swap), MS_SYNC);
	pthread_mutex_unlock(&memoria_swap_mtx);
	free(pagina);
}

int memoria_disponible_swap(){

	int frame_disponible = -1;
	int i = 0;

	pthread_mutex_lock(&frames_swap_mtx);
	while( i < cant_frames_swap && bitarray_test_bit(frames_swap, i)){
		i++;
	}

	if(i < cant_frames_swap && !bitarray_test_bit(frames_swap, i)){
		frame_disponible = i;
		bitarray_set_bit(frames_swap, i);
	}
	pthread_mutex_unlock(&frames_swap_mtx);

	return frame_disponible;
}

int memoria_disponible_mp(){

	int frame_disponible = -1;
	int i = 0;

	pthread_mutex_lock(&frames_MP_mtx);
	while(i < cant_frames_MP && bitarray_test_bit(frames_MP, i)){
		i++;
	}

	if(i < cant_frames_MP && !bitarray_test_bit(frames_MP, i)){
		frame_disponible = i;
		bitarray_set_bit(frames_MP, i);
	}



	pthread_mutex_unlock(&frames_MP_mtx);

	return frame_disponible;
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
	free_struct_mensaje(mensaje, FINALIZAR_PEDIDO);
	free(mensaje_a_procesar);
}

void liberar_pagina(t_pagina* pagina){
	pthread_mutex_lock(&frames_MP_mtx);
	bitarray_clean_bit(frames_MP, pagina->frame);
	pthread_mutex_unlock(&frames_MP_mtx);

	pagina->presencia = false;

	pthread_mutex_lock(&frames_swap_mtx);
	bitarray_clean_bit(frames_swap, pagina->pagina_swap);
	pthread_mutex_unlock(&frames_swap_mtx);
}

void free_pagina(t_pagina* pagina){ //TODO fijarnos si list_destroy elimina los elementos o no :o
	free(pagina);
}

void ejecucion_confirmar_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* mensaje = mensaje_a_procesar->mensaje;

	t_restaurante* restaurante = buscarRestaurante(mensaje->nombre.nombre);
	t_segmento* pedido;

	uint32_t confirmacion = 0;

	if(restaurante != NULL){
		pedido = buscarPedido(mensaje->id, restaurante);

		if(pedido != NULL){
			pthread_mutex_lock(&(restaurante->tabla_segmentos_mtx));
			if(pedido->estado == PENDIENTE){
				pedido->estado = CONFIRMADO;
				confirmacion = 1;
			}
			pthread_mutex_unlock(&(restaurante->tabla_segmentos_mtx));
		}
	}

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_CONFIRMAR_PEDIDO);
	free_struct_mensaje(mensaje, CONFIRMAR_PEDIDO);
	free(mensaje_a_procesar);

}

void ejecucion_plato_listo(t_mensaje_a_procesar* mensaje_a_procesar){
	m_platoListo* mensaje = mensaje_a_procesar->mensaje;

	t_restaurante* restaurante = buscarRestaurante(mensaje->restaurante.nombre);
	t_segmento* pedido;
	t_pagina* pagina = NULL;

	uint32_t confirmacion = 0;

	if(restaurante != NULL){
		pedido = buscarPedido(mensaje->idPedido, restaurante);

		if(pedido != NULL){
			pthread_mutex_lock(&(restaurante->tabla_segmentos_mtx));
			est_pedido estado_pedido = pedido->estado;
			pthread_mutex_unlock(&(restaurante->tabla_segmentos_mtx));

			if(estado_pedido == CONFIRMADO){
				pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
				pagina = buscarPlato(pedido->tabla_paginas, mensaje->comida.nombre);
				pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

				if(pagina != NULL){
					if(!pagina->presencia){
						traer_de_swap(pagina);
						pagina->presencia = true;
					}

					pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
					actualizar_plato_mp(pagina, 0, 1); //cantLista le suma 1
					pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

					confirmacion = 1;
				}else{
					puts("PAGINA ES NULL");
				}
			}else{
				puts("pedido no confirmado");

			}
		}else{
			puts("pedido es null");
		}
	}else{
		puts("no existe restaurante");
	}

	enviar_confirmacion(confirmacion, mensaje_a_procesar->socket_cliente, RTA_PLATO_LISTO);
	free_struct_mensaje(mensaje, PLATO_LISTO);
	free(mensaje_a_procesar);


}


void ejecucion_obtener_pedido(t_mensaje_a_procesar* mensaje_a_procesar){
	t_nombre_y_id* obtener_pedido = mensaje_a_procesar->mensaje;

	t_restaurante* restaurante = buscarRestaurante(obtener_pedido->nombre.nombre);
	t_segmento* pedido;

	t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));

	mensaje_a_enviar->id = config_get_int_value(config_comanda, ID_COMANDA);

	if(restaurante != NULL){
		pedido = buscarPedido(obtener_pedido->id, restaurante);

		if(pedido != NULL){

			t_pagina* pagina;
			rta_obtenerPedido* rtaObtenerPedido = malloc(sizeof(rta_obtenerPedido));

			rtaObtenerPedido->estadoPedido = pedido->estado;
			rtaObtenerPedido->infoPedidos = list_create();

			t_elemPedido* elemPedido;
			int offset;

			pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
			int cantidad_platos = pedido->tabla_paginas->elements_count;
			pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

			for(int i = 0; i < cantidad_platos; i++){

				void* plato_a_deserializar = malloc(TAMANIO_PAGINA);

				t_plato* plato;

				pthread_mutex_lock(&restaurante->tabla_segmentos_mtx);
				pagina = list_get(pedido->tabla_paginas, i);

				offset = pagina->frame * TAMANIO_PAGINA;

				pagina->ultimo_acceso = time(NULL);
				pagina->uso = true;

				if(!pagina->presencia){
					traer_de_swap(pagina);
				}

				pthread_mutex_unlock(&restaurante->tabla_segmentos_mtx);

				pthread_mutex_lock(&memoria_principal_mtx);
				memcpy(plato_a_deserializar, memoria_principal + offset, TAMANIO_PAGINA);
				pthread_mutex_unlock(&memoria_principal_mtx);

				plato = deserializar_pagina(plato_a_deserializar);

				elemPedido = malloc(sizeof(t_elemPedido));
				elemPedido->cantHecha = plato->cant_lista;
				elemPedido->cantTotal = plato->cant_pedida;
				elemPedido->comida.nombre = malloc(strlen(plato->nombre) + 1);
				strcpy(elemPedido->comida.nombre, plato->nombre);

				list_add(rtaObtenerPedido->infoPedidos, elemPedido);

				free(plato_a_deserializar);
				free(plato);

			}

			mensaje_a_enviar->tipo_mensaje = RTA_OBTENER_PEDIDO;
			mensaje_a_enviar->parametros = rtaObtenerPedido;

		}else{
			mensaje_a_enviar->tipo_mensaje = ERROR;
		}
	}else{
		mensaje_a_enviar->tipo_mensaje = ERROR;
	}

	enviar_mensaje(mensaje_a_enviar, mensaje_a_procesar->socket_cliente);
	loggear_mensaje_enviado(mensaje_a_enviar->parametros, mensaje_a_enviar->tipo_mensaje, log_comanda);

	free_struct_mensaje(mensaje_a_enviar->parametros, mensaje_a_enviar->tipo_mensaje);
	free(mensaje_a_enviar);
	free_struct_mensaje(obtener_pedido, OBTENER_PEDIDO);
	free(mensaje_a_procesar);
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

	return plato;
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
			free(plato);
			return pagina;
		}else{
			free(plato);
		}
	}
	return NULL;
}

