#include "utils_cliente.h"

bool validar_proceso(int argc, char** argv){
	return string_equals_ignore_case(argv[1], APP) ||
			string_equals_ignore_case(argv[1], COMANDA) ||
			string_equals_ignore_case(argv[1], RESTAURANTE) ||
			string_equals_ignore_case(argv[1], SINDICATO);
}

void configurar_ip_puerto(char* proceso){
	conexion = malloc(sizeof(t_conexion));
	char* puerto_proceso = string_new();
	string_append_with_format(&puerto_proceso,"PUERTO_%s",proceso);
	char* ip_proceso = string_new();
	string_append_with_format(&ip_proceso,"IP_%s",proceso);
	conexion->puerto = config_get_int_value(config_cliente, puerto_proceso);
	conexion->ip = config_get_string_value(config_cliente, ip_proceso);
	free(puerto_proceso);
	free(ip_proceso);
}

void iniciar_consola(){

	char* linea = readline(">");

	while(strncmp(linea, "", 1) != 0){
		if(linea){
			add_history(linea);
		}

		if(validar_mensaje(linea)){ //TODO ver si poner comando ayuda
			//hace cositas
		}else{
			puts("Por favor ingrese un mensaje valido");
		}

		free(linea);
		linea = readline(">");
	}
}

bool validar_mensaje(char* linea){

	char** mensaje_completo = string_split(linea, " ");

	char* proceso = mensaje_completo[0];
	char* tipo_mensaje = mensaje_completo[1];

	const bool tiene_argumentos_suficientes = !(mensaje_completo[0] == NULL || mensaje_completo[1] == NULL); //tiene que tener minimo 2 "palabras"
	const bool mensaje_valido = validar_proceso_mensaje(proceso,tipo_mensaje); //&& validar_argumentos(tipo_mensaje,mensaje_completo,proceso);

	return tiene_argumentos_suficientes && mensaje_valido;
}

bool validar_proceso_mensaje(char* proceso, char* tipo_mensaje){

	if(string_equals_ignore_case(proceso, APP)){
		return string_equals_ignore_case(MENSAJE_CONSULTAR_RESTAURANTES, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_SELECCIONAR_RESTAURANTE, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONSULTAR_PLATOS, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CREAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_AGREGAR_PLATO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONSULTAR_PEDIDO, tipo_mensaje);
	}else if(string_equals_ignore_case(proceso, COMANDA)){
		return string_equals_ignore_case(MENSAJE_GUARDAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_GUARDAR_PLATO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_OBTENER_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_PLATO_LISTO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_FINALIZAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje);
	}else if(string_equals_ignore_case(proceso, RESTAURANTE)){
		return string_equals_ignore_case(MENSAJE_CONSULTAR_PLATOS, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CREAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_AGREGAR_PLATO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONSULTAR_PEDIDO, tipo_mensaje);
	}else if(string_equals_ignore_case(proceso, SINDICATO)){
		return string_equals_ignore_case(MENSAJE_CONSULTAR_PLATOS, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_GUARDAR_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_GUARDAR_PLATO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_OBTENER_PEDIDO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_OBTENER_RESTAURANTE, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_PLATO_LISTO, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje);
	}
	return false;
}


void process_request(int cod_op, int cliente_fd){
	int size = 0;
	void* buffer = recibir_mensaje(cliente_fd, &size);
	puts("recibi un mensaje");

	m_seleccionarRestaurante * seleccionarRestaurante;
	uint32_t * id_confirmacion;
	t_nombre* nombre;
	t_nombre_y_id* nombre_id;
	m_guardarPlato* guardarPlato;
	m_platoListo* platoListo;
	t_restaurante_y_plato* restaurante_y_plato;
	rta_obtenerRestaurante* obtenerRestaurante;
	rta_consultarPedido* consultarPedido;
	rta_obtenerPedido* obtenerPedido;

	switch (cod_op) {
	case CONSULTAR_RESTAURANTES: //mensaje vacio
		puts("llego mensaje CONSULTAR_RESTAURANTES");
		break;
	case RTA_CONSULTAR_RESTAURANTES:
		puts("llego mensaje RTA_CONSULTAR_RESTAURANTES");
		restaurante_y_plato = deserializar_restaurante_y_plato(buffer);
		break;
	case SELECCIONAR_RESTAURANTE:
		puts("llego mensaje SELECCIONAR_RESTAURANTE");
		seleccionarRestaurante = deserializar_seleccionar_restaurante(buffer);
		break;

	case RTA_SELECCIONAR_RESTAURANTE:
		puts("llego mensaje RTA_SELECCIONAR_RESTAURANTE");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case OBTENER_RESTAURANTE:
		puts("llego mensaje OBTENER_RESTAURANTE");
		nombre = deserializar_nombre_restaurante(buffer);
		break;
	case RTA_OBTENER_RESTAURANTE:
		puts("llego mensaje RTA_OBTENER_RESTAURANTE");
		obtenerRestaurante = deserializar_rta_obtener_restaurante(buffer);
		break;
	case CONSULTAR_PLATOS:
		puts("llego mensaje CONSULTAR_PLATOS");
		nombre = deserializar_nombre_restaurante(buffer);
		break;
	case RTA_CONSULTAR_PLATOS:
		puts("llego mensaje RTA_CONSULTAR_PLATOS");
		deserializar_restaurante_y_plato(buffer);
		break;
	case CREAR_PEDIDO://vacio
		puts("llego mensaje CREAR_PEDIDO");
		break;
	case RTA_CREAR_PEDIDO:
		puts("llego mensaje RTA_CREAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case GUARDAR_PEDIDO:
		puts("llego mensaje GUARDAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_GUARDAR_PEDIDO:
		puts("llego mensaje RTA_GUARDAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case AGREGAR_PLATO:
		puts("llego mensaje AGREGAR_PLATO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_AGREGAR_PLATO:
		puts("llego mensaje RTA_AGREGAR_PLATO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case GUARDAR_PLATO:
		puts("llego mensaje GUARDAR_PLATO");
		guardarPlato = deserializar_guardar_plato(buffer);
		break;
	case RTA_GUARDAR_PLATO:
		puts("llego mensaje RTA_GUARDAR_PLATO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case CONFIRMAR_PEDIDO:
		puts("llego mensaje CONFIRMAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case RTA_CONFIRMAR_PEDIDO:
		puts("llego mensaje RTA_CONFIRMAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case PLATO_LISTO:
		puts("llego mensaje PLATO_LISTO");
		platoListo = deserializar_plato_listo(buffer);
		break;
	case RTA_PLATO_LISTO:
		puts("llego mensaje RTA_PLATO_LISTO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case CONSULTAR_PEDIDO:
		puts("llego mensaje CONSULTAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case RTA_CONSULTAR_PEDIDO:
		puts("llego mensaje RTA_CONSULTAR_PEDIDO");
		consultarPedido = deserializar_rta_consultar_pedido(buffer);
		break;
	case OBTENER_PEDIDO:
		puts("llego mensaje OBTENER_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_OBTENER_PEDIDO:
		puts("llego mensaje RTA_OBTENER_PEDIDO");
		obtenerPedido = deserializar_rta_obtener_pedido(buffer);
		break;
	case FINALIZAR_PEDIDO:
		puts("llego mensaje FINALIZAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_FINALIZAR_PEDIDO:
		puts("llego mensaje RTA_FINALIZAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	case TERMINAR_PEDIDO:
		puts("llego mensaje TERMINAR_PEDIDO");
		nombre_id = deserializar_nombre_y_id(buffer);
		break;
	case RTA_TERMINAR_PEDIDO:
		puts("llego mensaje RTA_TERMINAR_PEDIDO");
		id_confirmacion = deserializar_id_o_confirmacion(buffer);
		break;
	}
}

void recibir_mensajes_de_cola(int* socket){

	while(1){
		int cod_op;
		if(recv(*socket, &cod_op, sizeof(int), MSG_WAITALL) == -1)
			cod_op = -1;
		process_request(cod_op, *socket);
	}
}

void iniciarConexion(){

	int socket_broker = iniciar_servidor("127.0.0.1", "5001");
	recibir_mensajes_de_cola(&socket_broker);

	liberar_conexion(socket_broker);
}
