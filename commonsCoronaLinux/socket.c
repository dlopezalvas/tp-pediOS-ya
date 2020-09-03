#include "socket.h"


int iniciar_servidor (int puerto){
	struct sockaddr_in direccion_servidor;

	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = INADDR_ANY;
	direccion_servidor.sin_port = htons(puerto);

	int servidor = socket(AF_INET, SOCK_STREAM,0);

	//para poder probar que ande sin tener que esperar 2min
	int activado = 1;
	setsockopt(servidor, SOL_SOCKET, SO_REUSEADDR, &activado, sizeof(activado));

	if(bind(servidor, (void*) &direccion_servidor, sizeof(direccion_servidor)) !=0){
		perror("Fallo el bind");
	}


	listen(servidor,SOMAXCONN); //flag para que tome el maximo posible de espacio



	return servidor;

}



int iniciar_cliente(char* ip, int puerto){
	struct sockaddr_in direccion_servidor;

	direccion_servidor.sin_family = AF_INET;
	direccion_servidor.sin_addr.s_addr = inet_addr(ip);
	direccion_servidor.sin_port = htons(puerto);

	int cliente = socket(AF_INET, SOCK_STREAM, 0);

	if(connect(cliente, (void*) &direccion_servidor, sizeof(direccion_servidor)) !=0){
		perror("No se pudo conectar");
		return -1;
	}

	return cliente;
}

void* recibir_mensaje(int socket_cliente, int* size)
{
	void * buffer;

	int aux_size = 0;

	recv(socket_cliente, &aux_size, sizeof(int), 0);
	buffer = malloc(aux_size);
	recv(socket_cliente, buffer, aux_size, 0);

	*size = aux_size;

	return buffer;
}

void liberar_conexion(int socket_cliente){
	close(socket_cliente);
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

