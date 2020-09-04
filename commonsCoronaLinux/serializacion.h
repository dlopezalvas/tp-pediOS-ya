#ifndef serializacion_h
#define serializacion_h

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include "utils.h"

void enviar_mensaje(t_mensaje* mensaje, int socket);
void* serializar_paquete(t_paquete* paquete, int *bytes);
void* deserializar_mensaje(void* buffer, op_code tipo_mensaje);
t_buffer* cargar_buffer(t_mensaje* mensaje);
int tamanio_lista_strings(t_list* lista_de_strings);
int tamanio_lista_cocineroAfinidad(t_list* lista_de_strings);
int tamanio_lista_platos_con_estado(t_list* lista_de_platos);
int tamanio_lista_pedidos(t_list* lista_pedidos);

t_buffer* buffer_seleccionar_restaurante(m_seleccionarRestaurante* seleccionarRestaurante);
m_seleccionarRestaurante* deserializar_seleccionar_restaurante(void* buffer);
t_buffer* buffer_id_o_confirmacion(uint32_t* confirmacion);
uint32_t* deserializar_id_o_confirmacion(void* buffer);
t_buffer* buffer_nombre(t_nombre* nombre);
t_nombre* deserializar_nombre(void* buffer);
t_buffer* buffer_nombre_y_id(t_nombre_y_id* nombre_y_id);
t_nombre_y_id* deserializar_nombre_y_id(void* buffer);
t_buffer* buffer_guardar_plato(m_guardarPlato* guardar_plato);
m_guardarPlato* deserializar_guardar_plato(void* buffer);
t_buffer* buffer_plato_listo(m_platoListo* plato_listo);
m_platoListo* deserializar_plato_listo(void* buffer);
t_buffer* buffer_vacio();
t_buffer* buffer_restaurante_y_plato(t_restaurante_y_plato* lista_restaurantes);
t_restaurante_y_plato* deserializar_restaurante_y_plato(void* buffer);
t_buffer* buffer_rta_obtener_restaurante(rta_obtenerRestaurante* obtenerRestaurante);
rta_obtenerRestaurante* deserializar_rta_obtener_restaurante(void* buffer);
t_buffer* buffer_rta_consultar_pedido(rta_consultarPedido* consultarPedido);
rta_consultarPedido* deserializar_rta_consultar_pedido(void* buffer);
t_buffer* buffer_rta_obtener_pedido(rta_obtenerPedido* obtenerPedido);
rta_obtenerPedido* deserializar_rta_obtener_pedido(void* buffer);
t_buffer* buffer_posicion(t_coordenadas* posicion);
t_coordenadas* deserializar_posicion(void* buffer);


#endif /* serializacion_h */



