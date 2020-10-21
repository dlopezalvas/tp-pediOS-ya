#include "logs.h"

void loggear_mensaje_enviado(void* mensaje, op_code tipo_mensaje, t_log* logger){
	char* mensaje_string = t_mensaje_to_string(mensaje, tipo_mensaje);
	if(mensaje_string != NULL){
	log_info(logger, "Se envio el mensaje %s", mensaje_string);
	free(mensaje_string);
	}
}

void loggear_mensaje_recibido(void* mensaje, op_code tipo_mensaje, t_log* logger){
	char* mensaje_string = t_mensaje_to_string(mensaje, tipo_mensaje);
	if(mensaje_string != NULL){
	log_info(logger, "Se recibio el mensaje %s", mensaje_string);
	free(mensaje_string);
	}
}

char* t_mensaje_to_string(void* mensaje, op_code tipo_mensaje){

	struct_code tipo_struct = op_code_to_struct_code(tipo_mensaje);

	switch(tipo_struct){
	case STRC_MENSAJE_VACIO:return vacio_to_string(tipo_mensaje);

	case STRC_RESTAURANTE_Y_PLATO:return restaurante_y_plato_to_string(mensaje, tipo_mensaje);

	case STRC_SELECCIONAR_RESTAURANTE:return seleccionar_restaurante_to_string(mensaje, tipo_mensaje);

	case STRC_ID_CONFIRMACION: return id_o_confirmacion_to_string(mensaje, tipo_mensaje);

	case STRC_NOMBRE: return nombre_to_string(mensaje, tipo_mensaje);

	case STRC_RTA_OBTENER_RESTAURANTE:return rta_obtener_restaurante_to_string(mensaje, tipo_mensaje);

	case STRC_NOMBRE_ID: return nombre_y_id_to_string(mensaje, tipo_mensaje);

	case STRC_PLATO_LISTO:return plato_listo_to_string(mensaje, tipo_mensaje);

	case STRC_RTA_CONSULTAR_PEDIDO:return rta_consultar_pedido_to_string(mensaje, tipo_mensaje);

	case STRC_RTA_OBTENER_PEDIDO:return rta_obtener_pedido_to_string(mensaje, tipo_mensaje);

	case STRC_GUARDAR_PLATO:return guardar_plato_to_string(mensaje, tipo_mensaje);

	case STRC_RTA_OBTENER_RECETA:return rta_obtener_receta_to_string(mensaje, tipo_mensaje);
	}
	return NULL;

}

char* vacio_to_string(op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s", op_code_to_string(tipo_mensaje));
	return string_mensaje;
}

char* restaurante_y_plato_to_string(t_restaurante_y_plato* restaurante_plato, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s", op_code_to_string(tipo_mensaje));

	t_nombre* nombre;
	for(int i = 0; i<restaurante_plato->cantElementos; i++){
		nombre = list_get(restaurante_plato->nombres, i);
		string_append_with_format(&string_mensaje, " %s", nombre->nombre);
	}

	return string_mensaje;
}

char* seleccionar_restaurante_to_string(m_seleccionarRestaurante * seleccionar_restaurante, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s %d", op_code_to_string(tipo_mensaje),
			seleccionar_restaurante->restaurante.nombre, seleccionar_restaurante->cliente);
	return string_mensaje;
}

char* id_o_confirmacion_to_string(uint32_t* id_confirmacion, op_code tipo_mensaje){
	char* string_mensaje = string_new();

	int id = *id_confirmacion;

	if(es_id(tipo_mensaje)){
		string_append_with_format(&string_mensaje, "%s %d", op_code_to_string(tipo_mensaje), id);
	}else{
		string_append_with_format(&string_mensaje, "%s %s", op_code_to_string(tipo_mensaje),bool_to_string(id));
	}
	return string_mensaje;
}


bool es_id(op_code tipo_mensaje){
	return (tipo_mensaje == RTA_CREAR_PEDIDO ||	tipo_mensaje == CONFIRMAR_PEDIDO ||	tipo_mensaje == CONSULTAR_PEDIDO);
}

char* bool_to_string(bool confirmacion){
	if(confirmacion) return "OK";
	else return "FAIL";
}

char* nombre_to_string(t_nombre* nombre, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s", op_code_to_string(tipo_mensaje), nombre->nombre);
	return string_mensaje;
}

char* rta_obtener_restaurante_to_string(rta_obtenerRestaurante* obtener_restaurante, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s cantHornos %d x:%d y:%d ", op_code_to_string(tipo_mensaje),
			obtener_restaurante->cantHornos, obtener_restaurante->posicion.x, obtener_restaurante->posicion.y);
	char* afinidad;
	for(int i = 0; i<obtener_restaurante->cantCocineroAfinidad; i++){
		afinidad = list_get(obtener_restaurante->cocineroAfinidad, i);
		string_append_with_format(&string_mensaje, "cocinero %d %s ", i,afinidad);
	}
	string_append(&string_mensaje, "recetas: ");
	t_receta* receta;
	for(int j = 0; j<obtener_restaurante->recetas; j++){
		receta = list_get(obtener_restaurante->recetas, j);
		string_append_with_format(&string_mensaje, "%s - precio: %d", receta->receta.nombre, receta->precio);
	}
	return string_mensaje;
}

char* nombre_y_id_to_string(t_nombre_y_id * nombre_id, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s %d", op_code_to_string(tipo_mensaje),
			nombre_id->nombre.nombre, nombre_id->id);
	return string_mensaje;
}

char* plato_listo_to_string(m_platoListo * plato_listo, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s %d %s", op_code_to_string(tipo_mensaje),
			plato_listo->restaurante.nombre, plato_listo->idPedido, plato_listo->comida.nombre);
	return string_mensaje;
}

char* rta_consultar_pedido_to_string(rta_consultarPedido* consultar_pedido, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s %s", op_code_to_string(tipo_mensaje),
			consultar_pedido->restaurante, est_pedido_to_string(consultar_pedido->estadoPedido));
	t_elemPedido* plato;
	for(int i = 0; i<consultar_pedido->cantPlatos; i++){
		plato = list_get(consultar_pedido->platos, i);
		string_append_with_format(&string_mensaje, "%s - pedidos: %d listos: %d", plato->comida.nombre, plato->cantTotal, plato->cantHecha);
	}
	return string_mensaje;
}

char* est_pedido_to_string(est_pedido estado){
	switch(estado){
	case PENDIENTE:
		return "PENDIENTE";
	case CONFIRMADO:
		return "CONFIRMADO";
	case TERMINADO:
		return "TERMINADO";
	}
	return NULL;
}


char* rta_obtener_pedido_to_string(rta_obtenerPedido* obtener_pedido, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s estado: %s", op_code_to_string(tipo_mensaje), est_pedido_to_string(obtener_pedido->estadoPedido));
	t_elemPedido* elem_pedido;
	for(int i = 0; i<obtener_pedido->cantPedidos; i++){
		elem_pedido = list_get(obtener_pedido->infoPedidos, i);
		string_append_with_format(&string_mensaje, "%s listo:%d pedido:%d ", elem_pedido->comida.nombre,
				elem_pedido->cantHecha,elem_pedido->cantTotal);
	}
	return string_mensaje;
}


char* guardar_plato_to_string(m_guardarPlato* guardar_plato, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s %s id: %d %s cantidad: %d", op_code_to_string(tipo_mensaje), guardar_plato->restaurante.nombre,
			guardar_plato->idPedido, guardar_plato->comida.nombre, guardar_plato->cantidad);
	return string_mensaje;
}

char* rta_obtener_receta_to_string(rta_obtenerReceta* obtener_receta, op_code tipo_mensaje){
	char* string_mensaje = string_new();
	string_append_with_format(&string_mensaje, "%s ", op_code_to_string(tipo_mensaje));
	t_paso* paso;
	for(int i = 0; i<obtener_receta->cantPasos; i++){
		paso = list_get(obtener_receta->pasos, i);
		string_append_with_format(&string_mensaje, "%s duracion:%d", paso->paso.nombre, paso->duracion);
	}

	return string_mensaje;
}
