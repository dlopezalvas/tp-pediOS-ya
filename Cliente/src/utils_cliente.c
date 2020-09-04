#include "utils_cliente.h"

bool validar_proceso(int argc, char** argv){
	return string_equals_ignore_case(argv[1], APP) ||
			string_equals_ignore_case(argv[1], COMANDA) ||
			string_equals_ignore_case(argv[1], RESTAURANTE) ||
			string_equals_ignore_case(argv[1], SINDICATO);
}

void configurar_ip_puerto(){
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

	imprimir_mensajes_disponibles();

	char* linea = readline(">");

	while(strncmp(linea, "", 1) != 0){
		if(linea){
			add_history(linea);
		}

		if(string_equals_ignore_case(linea, COMANDO_HELP)){
			imprimir_mensajes_disponibles();
		}else if(validar_mensaje(linea)){
			t_mensaje* mensaje = llenarMensaje(linea);
			queue_push(mensajes_a_enviar, mensaje);
			sem_post(&sem_mensajes_a_enviar);
		}else{
			puts("Por favor ingrese un mensaje valido");
		}

		free(linea);
		linea = readline(">");
	}
}

void imprimir_mensajes_disponibles(){

	printf("Los mensajes disponibles para el proceso %s son:\n", proceso);

	if(strcmp(proceso, APP) == 0){
		printf("1-%s\n", FORMATO_CONSULTAR_RESTAURANTES);
		printf("2-%s\n", FORMATO_SELECCIONAR_RESTAURANTE);
		printf("3-%s\n", FORMATO_CONSULTAR_PLATOS);
		printf("4-%s\n", FORMATO_CREAR_PEDIDO);
		printf("5-%s\n", FORMATO_AGREGAR_PLATO);
		printf("6-%s\n", FORMATO_CONFIRMAR_PEDIDO);
		printf("7-%s\n", FORMATO_CONSULTAR_PEDIDO);
	}else if(strcmp(proceso, COMANDA) == 0){
		printf("1-%s\n", FORMATO_GUARDAR_PLATO);
		printf("2-%s\n", FORMATO_GUARDAR_PEDIDO);
		printf("3-%s\n", FORMATO_OBTENER_PEDIDO);
		printf("4-%s\n", FORMATO_PLATO_LISTO);
		printf("5-%s\n", FORMATO_FINALIZAR_PEDIDO);
		printf("6-%s\n", FORMATO_CONFIRMAR_PEDIDO);
	}else if(strcmp(proceso, RESTAURANTE) == 0){
		printf("1-%s\n", FORMATO_CONSULTAR_PLATOS);
		printf("2-%s\n", FORMATO_CREAR_PEDIDO);
		printf("3-%s\n", FORMATO_AGREGAR_PLATO);
		printf("4-%s\n", FORMATO_CONFIRMAR_PEDIDO);
		printf("5-%s\n", FORMATO_CONSULTAR_PEDIDO);
	}else if(strcmp(proceso, SINDICATO) == 0){
		printf("1-%s\n", FORMATO_CONSULTAR_PLATOS);
		printf("2-%s\n", FORMATO_GUARDAR_PEDIDO);
		printf("3-%s\n", FORMATO_GUARDAR_PLATO);
		printf("4-%s\n", FORMATO_OBTENER_PEDIDO);
		printf("5-%s\n", FORMATO_OBTENER_RESTAURANTE);
		printf("6-%s\n", FORMATO_CONFIRMAR_PEDIDO);
		printf("7-%s\n", FORMATO_PLATO_LISTO);
	}

	printf("Para consultar los mensajes ingrese %s\n", COMANDO_HELP);
}


bool validar_mensaje(char* linea){

	char** mensaje_completo = string_split(linea, " ");

	char* tipo_mensaje = mensaje_completo[0];

	const bool tiene_argumentos_suficientes = !(mensaje_completo[0] == NULL); //tiene que tener minimo 1 "palabras"
	const bool mensaje_valido = validar_proceso_mensaje(tipo_mensaje) && validar_argumentos(tipo_mensaje,mensaje_completo);

	return tiene_argumentos_suficientes && mensaje_valido;
}

bool validar_proceso_mensaje(char* tipo_mensaje){

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

bool validar_argumentos(char* tipo_mensaje, char** mensaje_completo){

	int cant_argumentos_mensaje = cantidad_argumentos(mensaje_completo);

	struct_code tipo_estructura = op_code_to_struct_code(string_to_op_code(tipo_mensaje));

	char** formato;

	switch(tipo_estructura){
	case STRC_MENSAJE_VACIO: return cant_argumentos_mensaje == 0;

	case STRC_SELECCIONAR_RESTAURANTE:
		formato = string_split(FORMATO_SELECCIONAR_RESTAURANTE, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato);

	case STRC_ID_CONFIRMACION:
		formato = string_split(FORMATO_CONFIRMAR_PEDIDO, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato); //como cuenta la cantidad de arg no importa cual mensaje sea

	case STRC_NOMBRE:
		formato = string_split(FORMATO_CONSULTAR_PLATOS, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato);

	case STRC_NOMBRE_ID:
		formato = string_split(FORMATO_GUARDAR_PEDIDO, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato);

	case STRC_PLATO_LISTO:
		formato = string_split(FORMATO_PLATO_LISTO, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato);

	case STRC_GUARDAR_PLATO:
		formato = string_split(FORMATO_GUARDAR_PLATO, " ");
		return cant_argumentos_mensaje == cantidad_argumentos(formato);

	}

	return false;

}

int cantidad_argumentos (char** mensaje_completo){
	int cantidad = 0;

	while(mensaje_completo[cantidad]!=NULL){
		cantidad++;
	}
	return cantidad - 1; //resto el proceso y el tipo de mensaje, quedan solo los argumentos
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
		nombre = deserializar_nombre(buffer);
		break;
	case RTA_OBTENER_RESTAURANTE:
		puts("llego mensaje RTA_OBTENER_RESTAURANTE");
		obtenerRestaurante = deserializar_rta_obtener_restaurante(buffer);
		break;
	case CONSULTAR_PLATOS:
		puts("llego mensaje CONSULTAR_PLATOS");
		nombre = deserializar_nombre(buffer);
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

void conexionEnvio(){
	int socket = iniciar_cliente("127.0.0.1", 5001);
	while(1){ //buscar condicion de que siga ejecutando
		sem_wait(&sem_mensajes_a_enviar);
		t_mensaje* mensaje = queue_pop(mensajes_a_enviar);
		enviar_mensaje(mensaje, socket);
		puts("envie el mensaje");
	}
}

t_mensaje* llenarMensaje(char* mensaje){

	char** parametros = string_split(mensaje, " ");

	op_code tipo_mensaje = string_to_op_code(parametros[0]);

	struct_code tipo_estructura = op_code_to_struct_code(tipo_mensaje);

	switch(tipo_estructura){
	case STRC_MENSAJE_VACIO: return llenar_vacio(parametros);

	case STRC_SELECCIONAR_RESTAURANTE:return llenar_seleccionar_restaurante(parametros);

	case STRC_ID_CONFIRMACION: return llenar_id_o_confirmacion(parametros);

	case STRC_NOMBRE: return llenar_nombre(parametros);

	case STRC_NOMBRE_ID: return llenar_nombre_y_id(parametros);

	case STRC_PLATO_LISTO:return llenar_plato_listo(parametros);

	case STRC_GUARDAR_PLATO:return llenar_guardar_plato(parametros);

	}
	return NULL;

}

t_mensaje* llenar_id_o_confirmacion(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);

	uint32_t* numero = malloc(sizeof(uint32_t));
	*numero = atoi(parametros[1]);;

	mensaje->parametros = numero;

	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_nombre_y_id(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	t_nombre_y_id* nombre_id = malloc(sizeof(t_nombre_y_id));
	nombre_id->nombre.nombre = string_duplicate(parametros[1]);
	nombre_id->id = atoi(parametros[2]);
	mensaje->parametros = nombre_id;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_nombre(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	t_nombre* nombre = malloc(sizeof(t_nombre));
	nombre->nombre = string_duplicate(parametros[1]);
	mensaje->parametros = nombre;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_guardar_plato(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	m_guardarPlato* guardarPlato = malloc(sizeof(m_guardarPlato));
	guardarPlato->restaurante.nombre = string_duplicate(parametros[1]);
	guardarPlato->idPedido = atoi(parametros[2]);
	guardarPlato->comida.nombre = string_duplicate(parametros[3]);
	guardarPlato->cantidad = atoi(parametros[4]);
	mensaje->parametros = guardarPlato;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_plato_listo(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	m_platoListo* guardarPlato = malloc(sizeof(m_platoListo));
	guardarPlato->restaurante.nombre = string_duplicate(parametros[1]);
	guardarPlato->idPedido = atoi(parametros[2]);
	guardarPlato->comida.nombre = string_duplicate(parametros[3]);
	mensaje->parametros = guardarPlato;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_restaurante_y_plato(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	t_restaurante_y_plato* restaurante_plato = malloc(sizeof(t_restaurante_y_plato));
	restaurante_plato->cantElementos = atoi(parametros[1]);
	restaurante_plato->nombres = list_create();
	for(int i= 0; i<restaurante_plato->cantElementos; i++){

		list_add(restaurante_plato->nombres, string_duplicate(parametros[i+2])); //ver si esto funciona
	}
	mensaje->parametros = restaurante_plato;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_seleccionar_restaurante(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	m_seleccionarRestaurante* seleccionarRestaurante = malloc(sizeof(m_seleccionarRestaurante));
	seleccionarRestaurante->restaurante.nombre = string_duplicate(parametros[1]);
	seleccionarRestaurante->cliente = atoi(parametros[2]);
	mensaje->parametros = seleccionarRestaurante;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}

t_mensaje* llenar_rta_obtener_restaurante(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	rta_obtenerRestaurante* obtenerRestaurante = malloc(sizeof(rta_obtenerRestaurante));
	obtenerRestaurante->posicion.x = atoi(parametros[1]);
	obtenerRestaurante->posicion.y = atoi(parametros[3]);
	obtenerRestaurante->cantRecetas = atoi(parametros[4]);
	obtenerRestaurante->recetas = list_create();
	int offset = 5;
	for(int i= 0; i<obtenerRestaurante->cantRecetas; i++){
		list_add(obtenerRestaurante->recetas, string_duplicate(parametros[i+offset])); //ver si esto funciona
	}
	offset += obtenerRestaurante->cantRecetas; //ver si necesita +1
	obtenerRestaurante->cantCocineroAfinidad =  atoi(parametros[offset]);
	offset ++;
	obtenerRestaurante->cocineroAfinidad = list_create();
	for(int j= 0; j<obtenerRestaurante->cantCocineroAfinidad && parametros[j+offset]; j++){

		list_add(obtenerRestaurante->cocineroAfinidad, string_duplicate(parametros[j+offset])); //ver si esto funciona
	}
	mensaje->parametros = obtenerRestaurante;
	//liberar_vector(parametros); creo que va aca
	return mensaje;
}


t_mensaje* llenar_vacio(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	//	liberar_vector(parametros);
	return mensaje;
}
