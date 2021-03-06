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
#include <commons/log.h>
#include <commons/string.h>
#include <commons/config.h>
#include <commons/collections/queue.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <semaphore.h>
#include <../commonsCoronaLinux/utils.h>
#include <../commonsCoronaLinux/socket.h>
#include <../commonsCoronaLinux/logs.h>

//Formatos

#define FORMATO_CONSULTAR_RESTAURANTES "CONSULTAR_RESTAURANTES"
#define FORMATO_SELECCIONAR_RESTAURANTE "SELECCIONAR_RESTAURANTE [RESTAURANTE]"
#define FORMATO_OBTENER_RESTAURANTE "OBTENER_RESTAURANTE [NOMBRE]"
#define FORMATO_CONSULTAR_PLATOS "CONSULTAR_PLATOS [NOMBRE]"
#define FORMATO_CREAR_PEDIDO "CREAR_PEDIDO"
#define FORMATO_GUARDAR_PEDIDO "GUARDAR_PEDIDO [NOMBRE] [ID]"
#define FORMATO_AGREGAR_PLATO "AGREGAR_PLATO [NOMBRE] [ID]"
#define FORMATO_CONFIRMAR_PEDIDO_CON_NOMBRE "CONFIRMAR_PEDIDO [NOMBRE] [ID]"
#define FORMATO_CONFIRMAR_PEDIDO_SIN_NOMBRE "CONFIRMAR_PEDIDO [ID]"
#define FORMATO_PLATO_LISTO "PLATO_LISTO [RESTAURANTE] [ID_PEDIDO] [COMIDA]"
#define FORMATO_CONSULTAR_PEDIDO "CONSULTAR_PEDIDO [ID]"
#define FORMATO_OBTENER_PEDIDO "OBTENER_PEDIDO [NOMBRE] [ID]"
#define FORMATO_FINALIZAR_PEDIDO "FINALIZAR_PEDIDO [NOMBRE] [ID]"
#define FORMATO_TERMINAR_PEDIDO "TERMINAR_PEDIDO [NOMBRE] [ID]"
#define FORMATO_GUARDAR_PLATO "GUARDAR_PLATO [RESTAURANTE] [ID_PEDIDO] [COMIDA] [CANTIDAD_COMIDA]"
#define FORMATO_OBTENER_RECETA "OBTENER_RECETA [NOMBRE]"

#define POSICION_X "POSICION_X"
#define POSICION_Y "POSICION_Y"
#define ID_CLIENTE "ID_CLIENTE"
#define PUERTO "PUERTO"
#define IP "IP"

#define COMANDO_HELP "HELP"
#define EXIT "EXIT"

typedef struct{
	int puerto;
	char* ip;
}t_conexion;

t_config* config_cliente;
t_log* log_cliente;
t_conexion* conexion;
char* proceso;
uint32_t id_cliente;

t_coordenadas posicion;

pthread_mutex_t iniciar_consola_mtx;

bool conexion_ok;

t_list* mensajes_hilos;

void iniciar_consola();
bool validar_proceso_mensaje(char* tipo_mensaje);
bool validar_argumentos(char* tipo_mensaje, char** mensaje_completo);
int cantidad_argumentos (char** mensaje_completo);
bool validar_mensaje(char* linea);
bool validar_proceso(char* proceso);
void seleccionar_proceso();
void configurar_ip_puerto();
void procesar_mensaje(t_mensaje* mensaje);
void conexionRecepcion();

void imprimir_mensajes_disponibles();

t_mensaje* llenarMensaje(char* mensaje);
t_mensaje* llenar_seleccionar_restaurante(char** parametros);
t_mensaje* llenar_id_o_confirmacion(char** parametros);
t_mensaje* llenar_nombre(char** parametros);
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
