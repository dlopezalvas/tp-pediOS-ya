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
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>


void recibir_mensajes_de_cola(int* socket){

	while(1){
		int cod_op;
		if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
			cod_op = -1;
		process_request(cod_op, *socket);
	}
}

void process_request(int cod_op, int cliente_fd){
	int size = 0;
	void* buffer = recibir_mensaje(cliente_fd, &size);
	puts("recibi un mensaje");

	m_seleccionarRestaurante * seleccionarRestaurante;
	uint32_t * id_confirmacion;
	t_nombre* nombre;
	t_nombre_y_id* nombre_id;
	m_guardarPlato* guardarPlato;
	m_platoListo* platoListo;
	t_restaurante_y_plato* restaurante_y_plato;
	rta_obtenerRestaurante* obtenerRestaurante;
	rta_consultarPedido* consultarPedido;
	rta_obtenerPedido* obtenerPedido;

	t_log* logger = log_create("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Comanda/comanda.LOG","comanda",0,LOG_LEVEL_INFO);

	switch (cod_op) {
	case CONSULTAR_RESTAURANTES: //mensaje vacio
		puts("llego mensaje CONSULTAR_RESTAURANTES");

		break;
	case RTA_CONSULTAR_RESTAURANTES:
		puts("llego mensaje RTA_CONSULTAR_RESTAURANTES");
		restaurante_y_plato = deserializar_restaurante_y_plato(buffer);
		break;
	case SELECCIONAR_RESTAURANTE:
		puts("llego mensaje SELECCIONAR_RESTAURANTE");
		seleccionarRestaurante = deserializar_seleccionar_restaurante(buffer);
		break;

	case RTA_SELECCIONAR_RESTAURANTE:
		puts("llego mensaje RTA_SELECCIONAR_RESTAURANTE");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case OBTENER_RESTAURANTE:
		puts("llego mensaje OBTENER_RESTAURANTE");
		nombre = deserializar_nombre(buffer);
		break;
	case RTA_OBTENER_RESTAURANTE:
		puts("llego mensaje RTA_OBTENER_RESTAURANTE");
		obtenerRestaurante = deserializar_rta_obtener_restaurante(buffer);
		break;
	case CONSULTAR_PLATOS:
		puts("llego mensaje CONSULTAR_PLATOS");
		nombre = deserializar_nombre(buffer);
		break;
	case RTA_CONSULTAR_PLATOS:
		puts("llego mensaje RTA_CONSULTAR_PLATOS");
		deserializar_restaurante_y_plato(buffer);
		break;
	case CREAR_PEDIDO://vacio
		puts("llego mensaje CREAR_PEDIDO");
		break;
	case RTA_CREAR_PEDIDO:
		puts("llego mensaje RTA_CREAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case GUARDAR_PEDIDO:
		puts("llego mensaje GUARDAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_GUARDAR_PEDIDO:
		puts("llego mensaje RTA_GUARDAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case AGREGAR_PLATO:
		puts("llego mensaje AGREGAR_PLATO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_AGREGAR_PLATO:
		puts("llego mensaje RTA_AGREGAR_PLATO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case GUARDAR_PLATO:
		puts("llego mensaje GUARDAR_PLATO");
		guardarPlato = deserializar_guardar_plato(buffer);
		break;
	case RTA_GUARDAR_PLATO:
		puts("llego mensaje RTA_GUARDAR_PLATO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case CONFIRMAR_PEDIDO:
		puts("llego mensaje CONFIRMAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case RTA_CONFIRMAR_PEDIDO:
		puts("llego mensaje RTA_CONFIRMAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case PLATO_LISTO:
		puts("llego mensaje PLATO_LISTO");
		platoListo = deserializar_plato_listo(buffer);
		break;
	case RTA_PLATO_LISTO:
		puts("llego mensaje RTA_PLATO_LISTO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case CONSULTAR_PEDIDO:
		puts("llego mensaje CONSULTAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case RTA_CONSULTAR_PEDIDO:
		puts("llego mensaje RTA_CONSULTAR_PEDIDO");
		consultarPedido = deserializar_rta_consultar_pedido(buffer);
		break;
	case OBTENER_PEDIDO:
		puts("llego mensaje OBTENER_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_OBTENER_PEDIDO:
		puts("llego mensaje RTA_OBTENER_PEDIDO");
		obtenerPedido = deserializar_rta_obtener_pedido(buffer);
		break;
	case FINALIZAR_PEDIDO:
		puts("llego mensaje FINALIZAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_FINALIZAR_PEDIDO:
		puts("llego mensaje RTA_FINALIZAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case TERMINAR_PEDIDO:
		puts("llego mensaje TERMINAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_TERMINAR_PEDIDO:
		puts("llego mensaje RTA_TERMINAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	}
}

int main(void) {

	iniciar_servidor(5001);



	return EXIT_SUCCESS;
}
