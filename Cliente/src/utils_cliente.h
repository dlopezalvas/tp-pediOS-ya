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

#define POSICION_X "POSICION_X"
#define POSICION_Y "POSICION_Y"

typedef struct{
	int puerto;
	char* ip;
}t_conexion;

t_config* config_cliente;
t_log* log_cliente;
t_conexion* conexion;
char* proceso;

void iniciar_consola();
bool validar_proceso_mensaje(char* tipo_mensaje);
bool validar_mensaje(char* linea);
bool validar_proceso(int argc, char** argv);
void configurar_ip_puerto();
void conexionEnvio();


t_mensaje* llenar_seleccionar_restaurante(char** parametros);
t_mensaje* llenar_id_o_confirmacion(char** parametros);
t_mensaje* llenar_nombre_restaurante(char** parametros);
t_mensaje* llenar_nombre_y_id(char** parametros);
t_mensaje* llenar_guardar_plato(char** parametros);
t_mensaje* llenar_plato_listo(char** parametros);
t_mensaje* llenar_restaurante_y_plato(char** parametros);
t_mensaje* llenar_rta_obtener_restaurante(char** parametros);
t_mensaje* llenar_rta_consultar_pedido(char** parametros);
t_mensaje* llenar_rta_obtener_pedido(char** parametros);
t_mensaje* llenar_vacio(char** parametros);


void process_request(int cod_op, int cliente_fd);
void recibir_mensajes_de_cola(int* socket);
void iniciarConexion();
#endif /* UTILS_CLIENTE_H_ */
