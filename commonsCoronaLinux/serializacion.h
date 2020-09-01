#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include<commons/collections/list.h>
#include "utils.h"

typedef struct{
	op_code tipo_mensaje;
	void* parametros; //ejemplo: ["PARAM1","PARAM2","PARAM3"]
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
} t_paquete;

typedef struct{
	uint32_t x;
	uint32_t y;
}t_coordenadas;

typedef struct{
	t_list* restaurantes; //lista de char* TODO ver si aca lo guardamos como t_nombre o char* (ultima opcion delega el largo solamente a la serializacion)
	uint32_t cantRestaurantes; //element count de restaurantes
}rta_consultarRestaurantes ;

typedef struct{
	uint32_t cliente;
	t_nombre restaurante;
}m_seleccionarRestaurante;

typedef struct{
	uint32_t idCocinero;
	t_nombre afinidad;
}t_cocineroAfinidad;

typedef struct{
	t_list* cocineroAfinidad; //lista de t_cocineroAfinidad: cocinero n con afinidad n
	t_coordenadas posicion;
	t_list* recetas; //lista de char*
	uint32_t cantidadHornos;
}rta_obtenerRestaurante;


typedef struct{
	t_list* platos; //lista de char*
}rta_consultarPlatos;

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
	pendiente = 1,
	confirmado = 2,
	terminado = 3,
}est_pedidos;

typedef struct{
	t_nombre restaurante;
	uint32_t idRepartidor;
	est_pedidos estadoPedido;
	t_list* platos; //ver si va con estado
}rta_consultarPedido;

typedef struct{    //mensajes obtener pedido, finalizar pedido, terminar pedido
	t_nombre nombre;
	uint32_t id;
}t_nombre_y_id;

typedef struct{
	t_nombre comida;
	uint32_t cantTotal;
	uint32_t cantHecha;
}elemPedido;

typedef struct{
	t_list* infoPedidos; //lista de elemPedido
}rta_obtenerPedido;


void enviar_mensaje(t_mensaje* mensaje, int socket);
void* serializar_paquete(t_paquete* paquete, int *bytes);
t_buffer* cargar_buffer(t_mensaje* mensaje);
int tamanio_lista_strings(t_list* lista_de_strings);

t_buffer* buffer_seleccionar_restaurante(m_seleccionarRestaurante* seleccionarRestaurante);
m_seleccionarRestaurante* deserializar_seleccionar_restaurante(void* buffer);
t_buffer* buffer_id_o_confirmacion(uint32_t* confirmacion);
uint32_t* deserealizar_id_o_confirmacion(void* buffer);
t_buffer* buffer_nombre_restaurante(t_nombre* nombre_restaurante);
t_nombre* deserealizar_nombre_restaurante(void* buffer);
t_buffer* buffer_nombre_y_id(t_nombre_y_id* nombre_y_id);
t_nombre_y_id* deserializar_nombre_y_id(void* buffer);
t_buffer* buffer_guardar_plato(m_guardarPlato* guardar_plato);
m_guardarPlato* deserializar_guardar_plato(void* buffer);
t_buffer* buffer_plato_listo(m_platoListo* plato_listo);
m_platoListo* deserializar_plato_listo(void* buffer);
t_buffer* buffer_vacio();
t_buffer* buffer_rta_consultar_restaurantes(rta_consultarRestaurantes* lista_restaurantes);








