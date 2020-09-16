/*
 ============================================================================
 Name        : Comanda.c
 Author      : 
 Version     :
 Copyright   : 
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>
t_log* logger;

void process_request(int cod_op, int cliente_fd){
	int size = 0;
	void* mensaje = NULL;
	if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
		void* buffer = recibir_mensaje(cliente_fd, &size);
		puts("recibi un mensaje");

		//	m_seleccionarRestaurante * seleccionarRestaurante;
		//	uint32_t * id_confirmacion;
		//	t_nombre* nombre;
		//	t_nombre_y_id* nombre_id;
		//	m_guardarPlato* guardarPlato;
		//	m_platoListo* platoListo;
		//	t_restaurante_y_plato* restaurante_y_plato;
		//	rta_obtenerRestaurante* obtenerRestaurante;
		//	rta_consultarPedido* consultarPedido;
		//	rta_obtenerPedido* obtenerPedido;
		mensaje = deserializar_mensaje(buffer, cod_op);
	}


	loggear_mensaje_recibido(mensaje, cod_op, logger);

	//	switch (cod_op) {
	//	case CONSULTAR_RESTAURANTES: //mensaje vacio
	//		puts("llego mensaje CONSULTAR_RESTAURANTES");
	//
	//		break;
	//	case RTA_CONSULTAR_RESTAURANTES:
	//		puts("llego mensaje RTA_CONSULTAR_RESTAURANTES");
	//		restaurante_y_plato = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case SELECCIONAR_RESTAURANTE:
	//		puts("llego mensaje SELECCIONAR_RESTAURANTE");
	//		seleccionarRestaurante = deserializar_mensaje(buffer, cod_op);
	//		break;
	//
	//	case RTA_SELECCIONAR_RESTAURANTE:
	//		puts("llego mensaje RTA_SELECCIONAR_RESTAURANTE");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case OBTENER_RESTAURANTE:
	//		puts("llego mensaje OBTENER_RESTAURANTE");
	//		nombre = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_OBTENER_RESTAURANTE:
	//		puts("llego mensaje RTA_OBTENER_RESTAURANTE");
	//		obtenerRestaurante = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case CONSULTAR_PLATOS:
	//		puts("llego mensaje CONSULTAR_PLATOS");
	//		nombre = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_CONSULTAR_PLATOS:
	//		puts("llego mensaje RTA_CONSULTAR_PLATOS");
	//		deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case CREAR_PEDIDO://vacio
	//		puts("llego mensaje CREAR_PEDIDO");
	//		break;
	//	case RTA_CREAR_PEDIDO:
	//		puts("llego mensaje RTA_CREAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case GUARDAR_PEDIDO:
	//		puts("llego mensaje GUARDAR_PEDIDO");
	//		nombre_id = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_GUARDAR_PEDIDO:
	//		puts("llego mensaje RTA_GUARDAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case AGREGAR_PLATO:
	//		puts("llego mensaje AGREGAR_PLATO");
	//		nombre_id = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_AGREGAR_PLATO:
	//		puts("llego mensaje RTA_AGREGAR_PLATO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case GUARDAR_PLATO:
	//		puts("llego mensaje GUARDAR_PLATO");
	//		guardarPlato = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_GUARDAR_PLATO:
	//		puts("llego mensaje RTA_GUARDAR_PLATO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case CONFIRMAR_PEDIDO:
	//		puts("llego mensaje CONFIRMAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_CONFIRMAR_PEDIDO:
	//		puts("llego mensaje RTA_CONFIRMAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case PLATO_LISTO:
	//		puts("llego mensaje PLATO_LISTO");
	//		platoListo = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_PLATO_LISTO:
	//		puts("llego mensaje RTA_PLATO_LISTO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case CONSULTAR_PEDIDO:
	//		puts("llego mensaje CONSULTAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_CONSULTAR_PEDIDO:
	//		puts("llego mensaje RTA_CONSULTAR_PEDIDO");
	//		consultarPedido = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case OBTENER_PEDIDO:
	//		puts("llego mensaje OBTENER_PEDIDO");
	//		nombre_id = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_OBTENER_PEDIDO:
	//		puts("llego mensaje RTA_OBTENER_PEDIDO");
	//		obtenerPedido = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case FINALIZAR_PEDIDO:
	//		puts("llego mensaje FINALIZAR_PEDIDO");
	//		nombre_id = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_FINALIZAR_PEDIDO:
	//		puts("llego mensaje RTA_FINALIZAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case TERMINAR_PEDIDO:
	//		puts("llego mensaje TERMINAR_PEDIDO");
	//		nombre_id = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	case RTA_TERMINAR_PEDIDO:
	//		puts("llego mensaje RTA_TERMINAR_PEDIDO");
	//		id_confirmacion = deserializar_mensaje(buffer, cod_op);
	//		break;
	//	}
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

	pthread_create(&hilo,NULL,(void*)serve_client,cliente);
	pthread_detach(hilo);

}

int main(void) {
	logger = log_create("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/comanda.LOG","comanda",1,LOG_LEVEL_INFO);

	int servidor = iniciar_servidor(5001);

	while(1){
		esperar_cliente(servidor);
	}

	return EXIT_SUCCESS;
}
