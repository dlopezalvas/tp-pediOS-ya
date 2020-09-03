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

char* cod_op_to_string(op_code tipo_mensaje){
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
		}
	return NULL;
}


op_code string_to_cod_op(char* tipo_mensaje){
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
	}else{
		return 0;
	}
}


