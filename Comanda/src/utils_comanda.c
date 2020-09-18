#include "utils_comanda.h"

void iniciar_comanda(){

	config_comanda = leer_config(PATH);

	hilos_clientes = list_create();
	pthread_mutex_init(&hilos_clientes_mtx, NULL);

	hilos_operaciones = list_create();
	pthread_mutex_init(&hilos_operaciones_mtx, NULL);

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

	pthread_mutex_lock(&hilos_operaciones_mtx);
	list_add(hilos_operaciones, &hilo_operacion);
	pthread_mutex_unlock(&hilos_operaciones_mtx);

	switch (cod_op) {
	case GUARDAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_guardar_pedido, mensaje);
		pthread_detach(hilo_operacion);
		break;
	case GUARDAR_PLATO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_guardar_plato, mensaje);
		pthread_detach(hilo_operacion);
		break;
	case CONFIRMAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_confirmar_pedido, mensaje);
		pthread_detach(hilo_operacion);
		break;
	case PLATO_LISTO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_plato_listo, mensaje);
		pthread_detach(hilo_operacion);
		break;
	case OBTENER_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_obtener_pedido, mensaje);
		pthread_detach(hilo_operacion);
		break;
	case FINALIZAR_PEDIDO:
		pthread_create(&hilo_operacion, NULL, (void*)ejecucion_finalizar_pedido, mensaje);
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

void ejecucion_guardar_pedido(){

}

void ejecucion_guardar_plato(){

}

void ejecucion_finalizar_pedido(){

}

void ejecucion_confirmar_pedido(){

}

void ejecucion_plato_listo(){

}

void ejecucion_obtener_pedido(){

}
