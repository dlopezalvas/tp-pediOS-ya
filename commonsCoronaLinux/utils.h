#ifndef UTILSCOMMONS_H_
#define UTILSCOMMONS_H_

#include<stdio.h>
#include<stdlib.h>
#include<signal.h>
#include<unistd.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<commons/log.h>
#include<commons/string.h>
#include<commons/config.h>


//Config
#define IP_APP "IP_APP"
#define IP_CLIENTE "IP_CLIENTE"
#define IP_COMANDA "IP_COMANDA"
#define IP_RESTAURANTE "IP_RESTAURANTE"
#define IP_SINDICATO "IP_SINDICATO"
#define PUERTO_APP "PUERTO_APP"
#define PUERTO_CLIENTE "PUERTO_CLIENTE"
#define PUERTO_COMANDA "PUERTO_COMANDA"
#define PUERTO_RESTAURANTE "PUERTO_RESTAURANTE"
#define PUERTO_SINDICATO "PUERTO_SINDICATO"


//Defino diferentes procesos
#define APP "APP"
#define CLIENTE "CLIENTE"
#define COMANDA "COMANDA"
#define RESTAURANTE "RESTAURANTE"
#define SINDICATO "SINDICATO"

typedef enum{
	CONSULTAR_RESTAURANTES = 1, //no recibe nada
	RTA_CONSULTAR_RESTAURANTES = 2,
	SELECCIONAR_RESTAURANTE = 3,
	RTA_SELECCIONAR_RESTAURANTE = 4, //Confirmacion Ok/Fail
	OBTENER_RESTAURANTE= 5,//recibe solo restaurante
	RTA_OBTENER_RESTAURANTE= 6,
	CONSULTAR_PLATOS = 7, //recibe solo restaurante
	RTA_CONSULTAR_PLATOS = 8,
	CREAR_PEDIDO = 9, //no recibe nada
	RTA_CREAR_PEDIDO = 10, //retorna idPedido
	GUARDAR_PEDIDO = 11, //m_pedido
	RTA_GUARDAR_PEDIDO = 12, //retorna idPedido
	AGREGAR_PLATO = 13,
	RTA_AGREGAR_PLATO  = 14, //Confirmacion Ok/Fail
	GUARDAR_PLATO = 15,
	RTA_GUARDAR_PLATO = 16, //Confirmacion Ok/Fail
	CONFIRMAR_PEDIDO = 17, //recibe idPedido
	RTA_CONFIRMAR_PEDIDO = 18, //Confirmacion Ok/Fail
	PLATO_LISTO = 19,
	RTA_PLATO_LISTO = 20, //Confirmacion Ok/Fail
	CONSULTAR_PEDIDO = 21, //recibe idPedido
	RTA_CONSULTAR_PEDIDO = 22,
	OBTENER_PEDIDO = 23, //m_pedido
	RTA_OBTENER_PEDIDO = 24,
	FINALIZAR_PEDIDO = 25, //m_pedido,
	RTA_FINALIZAR_PEDIDO = 26, //Confirmacion Ok/Fail
	TERMINAR_PEDIDO = 27, //m_pedido,
	RTA_TERMINAR_PEDIDO = 28, //Confirmacion Ok/Fail
}op_code;


t_log* iniciar_logger(t_config*);
t_config* leer_config(char* proceso);
void liberar_vector (char** vector);
op_code codigo_mensaje(char* tipo_mensaje);




#endif /* UTILS_H_ */

