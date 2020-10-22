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
#include<commons/collections/list.h>

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

//Mensajes
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
#define MENSAJE_OBTENER_RECETA "OBTENER_RECETA"

#define MENSAJE_RTA_CONSULTAR_RESTAURANTES "RTA_CONSULTAR_RESTAURANTES"
#define MENSAJE_RTA_SELECCIONAR_RESTAURANTE "RTA_SELECCIONAR_RESTAURANTE"
#define MENSAJE_RTA_OBTENER_RESTAURANTE "RTA_OBTENER_RESTAURANTE"
#define MENSAJE_RTA_CONSULTAR_PLATOS "RTA_CONSULTAR_PLATOS"
#define MENSAJE_RTA_CREAR_PEDIDO "RTA_CREAR_PEDIDO"
#define MENSAJE_RTA_GUARDAR_PEDIDO "RTA_GUARDAR_PEDIDO"
#define MENSAJE_RTA_AGREGAR_PLATO "RTA_AGREGAR_PLATO"
#define MENSAJE_RTA_CONFIRMAR_PEDIDO "RTA_CONFIRMAR_PEDIDO"
#define MENSAJE_RTA_PLATO_LISTO "RTA_PLATO_LISTO"
#define MENSAJE_RTA_CONSULTAR_PEDIDO "RTA_CONSULTAR_PEDIDO"
#define MENSAJE_RTA_OBTENER_PEDIDO "RTA_OBTENER_PEDIDO"
#define MENSAJE_RTA_FINALIZAR_PEDIDO "RTA_FINALIZAR_PEDIDO"
#define MENSAJE_RTA_TERMINAR_PEDIDO "RTA_TERMINAR_PEDIDO"
#define MENSAJE_RTA_GUARDAR_PLATO "RTA_GUARDAR_PLATO"
#define MENSAJE_RTA_OBTENER_RECETA "RTA_OBTENER_RECETA"

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
	POSICION_CLIENTE = 29,
	OBTENER_RECETA = 30,
	RTA_OBTENER_RECETA = 31,
	ERROR = 32,
	RTA_POSICION_CLIENTE = 33,
	POSICION_RESTAUNTE = 34
}op_code;

typedef enum{ //son para saber el tipo de struct que usa cada mensaje
	STRC_MENSAJE_VACIO = 0,
	STRC_RESTAURANTE_Y_PLATO = 1,
	STRC_SELECCIONAR_RESTAURANTE = 2,
	STRC_ID_CONFIRMACION = 3,
	STRC_NOMBRE = 4,
	STRC_RTA_OBTENER_RESTAURANTE = 5,
	STRC_NOMBRE_ID = 6,
	STRC_PLATO_LISTO = 7,
	STRC_RTA_CONSULTAR_PEDIDO =8,
	STRC_RTA_OBTENER_PEDIDO =9,
	STRC_GUARDAR_PLATO = 10,
	STRC_POSICION = 11,
	STRC_RTA_OBTENER_RECETA = 12,
	STRC_POSICION_RESTAUNTE = 13
}struct_code;



typedef struct{
	op_code tipo_mensaje;
	uint32_t id;
	void* parametros; //struct del tipo de mensaje
}t_mensaje;


typedef struct{
	uint32_t size;
	void* stream;
} t_buffer;

typedef struct{
	char* nombre;
	uint32_t largo_nombre;
}t_nombre;

typedef struct{
	op_code codigo_operacion;
	t_buffer* buffer;
	uint32_t id;
} t_paquete;

typedef struct{
	uint32_t x;
	uint32_t y;
}t_coordenadas;

typedef struct{

	t_nombre nombre;
	t_coordenadas posicion;
} m_restaurante;

typedef struct{
	t_list* nombres; //lista de t_nombre
	uint32_t cantElementos; //element count de los nombres
}t_restaurante_y_plato;

typedef struct{
	uint32_t cliente;
	t_nombre restaurante;
}m_seleccionarRestaurante;

typedef struct{
	uint32_t idCocinero;
	t_nombre afinidad;
}t_cocineroAfinidad;

typedef struct{
	uint32_t cantAfinidades;
	t_list* afinidades; //lista de t_nombre
	t_coordenadas posicion;
	uint32_t cantRecetas;
	t_list* recetas; //lista de receta con precio
	uint32_t cantHornos;
	uint32_t cantCocineros;
}rta_obtenerRestaurante;


typedef struct{
	t_nombre receta;
	uint32_t precio;
}t_receta;

typedef struct{
	t_nombre restaurante;
	uint32_t idPedido;
	t_nombre comida; //plato?
	uint32_t cantidad;
}m_guardarPlato;

typedef struct{
	t_nombre restaurante;
	uint32_t idPedido;
	t_nombre comida; //plato?
}m_platoListo;

typedef enum{
	PENDIENTE = 1,
	CONFIRMADO = 2,
	TERMINADO = 3,
}est_pedido;

//typedef enum{
//	LISTO = 1,
//	EN_PROCESO = 2,
//}est_plato;

//typedef struct{
//	t_nombre plato;
//	est_plato estadoPlato;
//}t_plato_con_estado;

typedef struct{
	t_nombre restaurante;
	est_pedido estadoPedido;
	uint32_t cantPlatos;
	t_list* platos; //lista elem pedido
}rta_consultarPedido;

typedef struct{    //mensajes obtener pedido, finalizar pedido, terminar pedido
	t_nombre nombre;
	uint32_t id;
}t_nombre_y_id;

typedef struct{
	t_nombre comida;
	uint32_t cantTotal;
	uint32_t cantHecha;
}t_elemPedido;

typedef struct{
	est_pedido estadoPedido;
	uint32_t cantPedidos;
	t_list* infoPedidos; //lista de elemPedido
}rta_obtenerPedido;

typedef struct{
	uint32_t cantPasos;
	t_list* pasos;
}rta_obtenerReceta;

typedef struct{
	t_nombre paso;
	uint32_t duracion;
}t_paso;

t_log* iniciar_logger(t_config*);
t_config* leer_config(char* proceso);
void liberar_vector (char** vector);
char* op_code_to_string(op_code tipo_mensaje);
op_code string_to_op_code(char* tipo_mensaje);
struct_code op_code_to_struct_code(op_code tipo_mensaje);

void free_struct_mensaje(void* mensaje, op_code tipo_mensaje);
void free_restaurante_y_plato(t_restaurante_y_plato* mensaje);
void free_seleccionar_restaurante(m_seleccionarRestaurante* mensaje);
void free_id_o_confirmacion(uint32_t* mensaje);
void free_nombre(t_nombre* mensaje);
void free_rta_obtener_restaurante(rta_obtenerRestaurante* mensaje);
void free_receta(t_receta* receta);
void free_cocineroAfinidad(t_cocineroAfinidad* cocineroAfinidad);
void free_nombre_y_id(t_nombre_y_id* mensaje);
void free_plato_listo(m_platoListo* mensaje);
void free_rta_consultar_pedido(rta_consultarPedido* mensaje);

void free_rta_obtener_pedido(rta_obtenerPedido* mensaje);
void free_infoPedidos(t_elemPedido* info_pedido);
void free_guardar_plato(m_guardarPlato* mensaje);
void free_posicion(t_coordenadas* mensaje);
void free_rta_obtener_receta(rta_obtenerReceta* mensaje);
void free_pasos(t_paso* paso);
void free_posicion_restaurante(m_restaurante* mensaje);



#endif /* UTILS_H_ */

