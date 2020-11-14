#ifndef LOGS_H_
#define LOGS_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include "utils.h"

t_log* iniciar_logger(t_config*);
char* t_mensaje_to_string(void* mensaje, op_code tipo_mensaje);
void loggear_mensaje_enviado(void* mensaje, op_code tipo_mensaje, t_log* logger);
void loggear_mensaje_recibido(void* mensaje, op_code tipo_mensaje, t_log* logger);
char* vacio_to_string(op_code tipo_mensaje);
char* restaurante_y_plato_to_string(t_restaurante_y_plato* restaurante_plato, op_code tipo_mensaje);
char* seleccionar_restaurante_to_string(m_seleccionarRestaurante * seleccionar_restaurante, op_code tipo_mensaje);
char* id_o_confirmacion_to_string(uint32_t * id_confirmacion, op_code tipo_mensaje);
bool es_id(op_code tipo_mensaje);
char* confirmacion_to_string(t_confirmacion confirmacion);
char* nombre_to_string(t_nombre* nombre, op_code tipo_mensaje);
char* rta_obtener_restaurante_to_string(rta_obtenerRestaurante* obtener_restaurante, op_code tipo_mensaje);
char* nombre_y_id_to_string(t_nombre_y_id * nombre_id, op_code tipo_mensaje);
char* plato_listo_to_string(m_platoListo * plato_listo, op_code tipo_mensaje);
char* rta_consultar_pedido_to_string(rta_consultarPedido* consultar_pedido, op_code tipo_mensaje);
char* est_pedido_to_string(est_pedido estado);
char* rta_obtener_pedido_to_string(rta_obtenerPedido* obtener_pedido, op_code tipo_mensaje);
char* guardar_plato_to_string(m_guardarPlato* guardar_plato, op_code tipo_mensaje);
char* rta_obtener_receta_to_string(rta_obtenerReceta* obtener_receta, op_code tipo_mensaje);


#endif /* LOGS_H_ */
