#include "utils.h"

t_log* iniciar_logger(t_config* config)
{
	char* nombre_archivo = config_get_string_value(config,"LOG_FILE");
	char* nombre_aplicacion = config_get_string_value(config,"NOMBRE_APLICACION");
	//char* path = config_get_string_value(config,"PATH");
	int log_consola = config_get_int_value(config,"LOG_CONSOLA");

	t_log* logger = log_create(nombre_archivo,nombre_aplicacion,log_consola,LOG_LEVEL_INFO);
	return logger;
}

t_config* leer_config(char* path)
{
	t_config* config = config_create(path);
	return config;
}



void liberar_vector (char** vector){

	int i = 0;
	while(vector[i]!=NULL){
		free(vector[i]);
		i++;
	}

	free(vector);
}

char* op_code_to_string(op_code tipo_mensaje){
	switch (tipo_mensaje) {
	case CONSULTAR_RESTAURANTES:
		return MENSAJE_CONSULTAR_RESTAURANTES;
		break;
	case RTA_CONSULTAR_RESTAURANTES:
		return MENSAJE_RTA_CONSULTAR_RESTAURANTES;
		break;
	case SELECCIONAR_RESTAURANTE:
		return MENSAJE_SELECCIONAR_RESTAURANTE;
		break;
	case RTA_SELECCIONAR_RESTAURANTE:
		return MENSAJE_RTA_SELECCIONAR_RESTAURANTE;
		break;
	case OBTENER_RESTAURANTE:
		return MENSAJE_OBTENER_RESTAURANTE;
		break;
	case RTA_OBTENER_RESTAURANTE:
		return MENSAJE_RTA_OBTENER_RESTAURANTE;
		break;
	case CONSULTAR_PLATOS:
		return MENSAJE_CONSULTAR_PLATOS;
		break;
	case RTA_CONSULTAR_PLATOS:
		return MENSAJE_RTA_CONSULTAR_PLATOS;
		break;
	case CREAR_PEDIDO:
		return MENSAJE_CREAR_PEDIDO;
		break;
	case RTA_CREAR_PEDIDO:
		return MENSAJE_RTA_CREAR_PEDIDO;
		break;
	case GUARDAR_PEDIDO:
		return MENSAJE_GUARDAR_PEDIDO;
		break;
	case RTA_GUARDAR_PEDIDO:
		return MENSAJE_RTA_GUARDAR_PEDIDO;
		break;
	case AGREGAR_PLATO:
		return MENSAJE_AGREGAR_PLATO;
		break;
	case RTA_AGREGAR_PLATO:
		return MENSAJE_RTA_AGREGAR_PLATO;
		break;
	case GUARDAR_PLATO:
		return MENSAJE_GUARDAR_PLATO;
		break;
	case RTA_GUARDAR_PLATO:
		return MENSAJE_RTA_GUARDAR_PLATO;
		break;
	case CONFIRMAR_PEDIDO:
		return MENSAJE_CONFIRMAR_PEDIDO;
		break;
	case RTA_CONFIRMAR_PEDIDO:
		return MENSAJE_RTA_CONFIRMAR_PEDIDO;
		break;
	case PLATO_LISTO:
		return MENSAJE_PLATO_LISTO;
		break;
	case RTA_PLATO_LISTO:
		return MENSAJE_RTA_PLATO_LISTO;
		break;
	case CONSULTAR_PEDIDO:
		return MENSAJE_CONSULTAR_PEDIDO;
		break;
	case RTA_CONSULTAR_PEDIDO:
		return MENSAJE_RTA_CONSULTAR_PEDIDO;
		break;
	case OBTENER_PEDIDO:
		return MENSAJE_OBTENER_PEDIDO;
		break;
	case RTA_OBTENER_PEDIDO:
		return MENSAJE_RTA_OBTENER_PEDIDO;
		break;
	case FINALIZAR_PEDIDO:
		return MENSAJE_FINALIZAR_PEDIDO;
		break;
	case RTA_FINALIZAR_PEDIDO:
		return MENSAJE_RTA_FINALIZAR_PEDIDO;
		break;
	case TERMINAR_PEDIDO:
		return MENSAJE_TERMINAR_PEDIDO;
		break;
	case RTA_TERMINAR_PEDIDO:
		return MENSAJE_RTA_TERMINAR_PEDIDO;
		break;
	case OBTENER_RECETA:
		return MENSAJE_OBTENER_RECETA;
		break;
	case RTA_OBTENER_RECETA:
		return MENSAJE_RTA_OBTENER_RECETA;
		break;
	case ERROR:
		return MENSAJE_ERROR;
	}
	return NULL;
}


op_code string_to_op_code(char* tipo_mensaje){
	if(string_equals_ignore_case(MENSAJE_CONSULTAR_RESTAURANTES, tipo_mensaje)){
		return CONSULTAR_RESTAURANTES;
	}else if(string_equals_ignore_case(MENSAJE_SELECCIONAR_RESTAURANTE, tipo_mensaje)){
		return SELECCIONAR_RESTAURANTE;
	}else if(string_equals_ignore_case(MENSAJE_OBTENER_RESTAURANTE, tipo_mensaje)){
		return OBTENER_RESTAURANTE;
	}else if(string_equals_ignore_case(MENSAJE_CONSULTAR_PLATOS, tipo_mensaje)){
		return CONSULTAR_PLATOS;
	}else if(string_equals_ignore_case(MENSAJE_CREAR_PEDIDO, tipo_mensaje)){
		return CREAR_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_GUARDAR_PEDIDO, tipo_mensaje)){
		return GUARDAR_PEDIDO;
	}else	if(string_equals_ignore_case(MENSAJE_AGREGAR_PLATO, tipo_mensaje)){
		return AGREGAR_PLATO;
	}else if(string_equals_ignore_case(MENSAJE_GUARDAR_PLATO, tipo_mensaje)){
		return GUARDAR_PLATO;
	}else if(string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje)){
		return CONFIRMAR_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_PLATO_LISTO, tipo_mensaje)){
		return PLATO_LISTO;
	}else if(string_equals_ignore_case(MENSAJE_CONSULTAR_PEDIDO, tipo_mensaje)){
		return CONSULTAR_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_OBTENER_PEDIDO, tipo_mensaje)){
		return OBTENER_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_FINALIZAR_PEDIDO, tipo_mensaje)){
		return FINALIZAR_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_TERMINAR_PEDIDO, tipo_mensaje)){
		return TERMINAR_PEDIDO;
	}else if(string_equals_ignore_case(MENSAJE_OBTENER_RECETA, tipo_mensaje)){
		return OBTENER_RECETA;
	}else{
		return 0;
	}
}

struct_code op_code_to_struct_code(op_code tipo_mensaje){
	if(tipo_mensaje == RTA_SELECCIONAR_RESTAURANTE ||
			tipo_mensaje == RTA_CREAR_PEDIDO ||
			tipo_mensaje == RTA_GUARDAR_PEDIDO ||
			tipo_mensaje == RTA_AGREGAR_PLATO ||
			tipo_mensaje == RTA_GUARDAR_PLATO ||
			tipo_mensaje == RTA_CONFIRMAR_PEDIDO ||
			tipo_mensaje == RTA_PLATO_LISTO ||
			tipo_mensaje == CONSULTAR_PEDIDO ||
			tipo_mensaje == RTA_FINALIZAR_PEDIDO ||
			tipo_mensaje == RTA_POSICION_CLIENTE ||
			tipo_mensaje == RTA_TERMINAR_PEDIDO) return STRC_ID_CONFIRMACION;
	else if(tipo_mensaje == GUARDAR_PEDIDO ||
			tipo_mensaje == OBTENER_PEDIDO ||
			tipo_mensaje == FINALIZAR_PEDIDO ||
			tipo_mensaje == CONFIRMAR_PEDIDO ||
			tipo_mensaje == TERMINAR_PEDIDO ||
			tipo_mensaje == AGREGAR_PLATO) return STRC_NOMBRE_ID;
	else if(tipo_mensaje == OBTENER_RESTAURANTE ||
			tipo_mensaje == OBTENER_RECETA ||
			tipo_mensaje == CONSULTAR_PLATOS) return STRC_NOMBRE;
	else if(tipo_mensaje == CONSULTAR_RESTAURANTES ||
			tipo_mensaje == ERROR ||
			tipo_mensaje == CREAR_PEDIDO) return STRC_MENSAJE_VACIO;
	else if(tipo_mensaje == RTA_CONSULTAR_RESTAURANTES ||
			tipo_mensaje == RTA_CONSULTAR_PLATOS) return STRC_RESTAURANTE_Y_PLATO;
	else if(tipo_mensaje == SELECCIONAR_RESTAURANTE) return STRC_SELECCIONAR_RESTAURANTE;
	else if(tipo_mensaje == GUARDAR_PLATO) return STRC_GUARDAR_PLATO;
	else if(tipo_mensaje == PLATO_LISTO) return STRC_PLATO_LISTO;
	else if(tipo_mensaje == RTA_CONSULTAR_PEDIDO) return STRC_RTA_CONSULTAR_PEDIDO;
	else if(tipo_mensaje == RTA_OBTENER_PEDIDO) return STRC_RTA_OBTENER_PEDIDO;
	else if(tipo_mensaje == RTA_OBTENER_RESTAURANTE) return STRC_RTA_OBTENER_RESTAURANTE;
	else if(tipo_mensaje == POSICION_CLIENTE) return STRC_POSICION;
	else if(tipo_mensaje == RTA_OBTENER_RECETA) return STRC_RTA_OBTENER_RECETA;
	else if(tipo_mensaje == POSICION_RESTAUNTE) return STRC_POSICION_RESTAUNTE;
	return -1;
}

//Free mensajes


void free_struct_mensaje(void* mensaje, op_code tipo_mensaje){

	struct_code tipo_struct = op_code_to_struct_code(tipo_mensaje);

	switch(tipo_struct){
	case STRC_MENSAJE_VACIO:
		break;
	case STRC_RESTAURANTE_Y_PLATO:
		free_restaurante_y_plato(mensaje);
		break;
	case STRC_SELECCIONAR_RESTAURANTE:
		free_seleccionar_restaurante(mensaje);
		break;
	case STRC_ID_CONFIRMACION:
		free_id_o_confirmacion(mensaje);
		break;
	case STRC_NOMBRE:
		free_nombre(mensaje);
		break;
	case STRC_RTA_OBTENER_RESTAURANTE:
		free_rta_obtener_restaurante(mensaje);
		break;
	case STRC_NOMBRE_ID:
		free_nombre_y_id(mensaje);
		break;
	case STRC_PLATO_LISTO:
		free_plato_listo(mensaje);
		break;
	case STRC_RTA_CONSULTAR_PEDIDO:
		free_rta_consultar_pedido(mensaje);
		break;
	case STRC_RTA_OBTENER_PEDIDO:
		free_rta_obtener_pedido(mensaje);
		break;
	case STRC_GUARDAR_PLATO:
		free_guardar_plato(mensaje);
		break;
	case STRC_POSICION:
		free_posicion(mensaje);
		break;
	case STRC_RTA_OBTENER_RECETA:
		free_rta_obtener_receta(mensaje);
		break;
	case STRC_POSICION_RESTAUNTE:
		free_posicion_restaurante(mensaje);
	}
}

void free_rta_obtener_receta(rta_obtenerReceta* mensaje){
	list_destroy_and_destroy_elements(mensaje->pasos, (void*) free_pasos);
	free(mensaje);
}

void free_pasos(t_paso* paso){
	free(paso->paso.nombre);
	free(paso);
}

void free_restaurante_y_plato(t_restaurante_y_plato* mensaje){
	list_destroy(mensaje->nombres);
	free(mensaje);
}

void free_seleccionar_restaurante(m_seleccionarRestaurante* mensaje){
	free(mensaje->restaurante.nombre);
	free(mensaje);
}

void free_posicion_restaurante(m_restaurante* mensaje){
	free(mensaje->nombre.nombre);
	free(mensaje);
}

void free_id_o_confirmacion(uint32_t* mensaje){
	free(mensaje);
}

void free_nombre(t_nombre* mensaje){
	free(mensaje->nombre);
	free(mensaje);
}

void free_rta_obtener_restaurante(rta_obtenerRestaurante* mensaje){
	list_destroy_and_destroy_elements(mensaje->afinidades, (void*) free_nombre);
	list_destroy_and_destroy_elements(mensaje->recetas, (void*) free_receta);
	free(mensaje);
}

void free_receta(t_receta* receta){
	free(receta->receta.nombre);
	free(receta);
}

void free_nombre_y_id(t_nombre_y_id* mensaje){
	free(mensaje->nombre.nombre);
	free(mensaje);
}

void free_plato_listo(m_platoListo* mensaje){
	free(mensaje->comida.nombre);
	free(mensaje->restaurante.nombre);
	free(mensaje);
}

void free_rta_consultar_pedido(rta_consultarPedido* mensaje){
	list_destroy_and_destroy_elements(mensaje->platos, (void*) free_infoPedidos);
	free(mensaje);
}

void free_rta_obtener_pedido(rta_obtenerPedido* mensaje){
	list_destroy_and_destroy_elements(mensaje->infoPedidos, (void*) free_infoPedidos);
	free(mensaje);
}

void free_infoPedidos(t_elemPedido* info_pedido){
	free(info_pedido->comida.nombre);
	free(info_pedido);
}

void free_guardar_plato(m_guardarPlato* mensaje){
	free(mensaje->comida.nombre);
	free(mensaje->restaurante.nombre);
	free(mensaje);
}

void free_posicion(t_coordenadas* mensaje){
	free(mensaje);
}
