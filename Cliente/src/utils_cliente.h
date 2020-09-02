/*
 * utils_cliente.h
 *
 *  Created on: 1 sep. 2020
 *      Author: utnso
 */

#ifndef UTILS_CLIENTE_H_
#define UTILS_CLIENTE_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>
#include<readline/readline.h>
#include<readline/history.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>


#define MENSAJE_CONSULTAR_RESTAURANTES "CONSULTAR_RESTAURANTES"
#define MENSAJE_SELECCIONAR_RESTAURANTE "SELECCIONAR_RESTAURANTE"
#define MENSAJE_OBTENER_RESTAURANTE "OBTENER_RESTAURANTE"
#define MENSAJE_CONSULTAR_PLATOS "CONSULTAR_PLATOS"
#define MENSAJE_CREAR_PEDIDO "CREAR_PEDIDO"
#define MENSAJE_GUARDAR_PEDIDO "GUARDAR_PEDIDO"
#define MENSAJE_AGREGAR_PLATO "AGREGAR_PLATO"
#define MENSAJE_CONFIRMAR_PEDIDO "CONFIRMAR_PEDIDO"
#define MENSAJE_PLATO_LISTO "PLATO_LISTO"
#define MENSAJE_CONSULTAR_PEDIDO "CONSULTAR_PEDIDO"
#define MENSAJE_OBTENER_PEDIDO "OBTENER_PEDIDO"
#define MENSAJE_FINALIZAR_PEDIDO "FINALIZAR_PEDIDO"
#define MENSAJE_TERMINAR_PEDIDO "TERMINAR_PEDIDO"
#define MENSAJE_GUARDAR_PLATO "GUARDAR_PLATO"

#define POSICION_X "POSICION_X"
#define POSICION_Y "POSICION_Y"

t_config* config_cliente;
t_log* log_cliente;

typedef struct{ //TODO ver que tenemos ahora un atoi para el puerto, conviene directamente sacarlo y poner como int
	char* ip_comanda;
	char* puerto_comanda;
	char* ip_restaurante;
	char* puerto_restaurante;
	char* ip_app;
	char* puerto_app;
	char* ip_sindicato;
	char* puerto_sindicato;
	t_coordenadas* posicion;
}config_datos;

config_datos* s_configuracion_cliente;

void cargar_configuracion();

void iniciar_consola();
bool validar_proceso_mensaje(char* proceso, char* tipo_mensaje);
bool validar_mensaje(char* linea);

void process_request(int cod_op, int cliente_fd);
void recibir_mensajes_de_cola(int* socket);
void iniciarConexion();
#endif /* UTILS_CLIENTE_H_ */
