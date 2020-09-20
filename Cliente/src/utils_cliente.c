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

	pthread_mutex_lock(&iniciar_consola_mtx);

	if(conexion_ok){

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
		free(linea);
	}

	return;
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
		printf("8-%s\n", FORMATO_OBTENER_RECETA);
	}

	printf("Para consultar los mensajes ingrese %s\n", COMANDO_HELP);
}


bool validar_mensaje(char* linea){

	char** mensaje_completo = string_split(linea, " ");

	char* tipo_mensaje = mensaje_completo[0];

	const bool tiene_argumentos_suficientes = !(mensaje_completo[0] == NULL); //tiene que tener minimo 1 "palabras"
	const bool mensaje_valido = validar_proceso_mensaje(tipo_mensaje) && validar_argumentos(tipo_mensaje,mensaje_completo);

	liberar_vector(mensaje_completo);

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
				string_equals_ignore_case(MENSAJE_OBTENER_RECETA, tipo_mensaje) ||
				string_equals_ignore_case(MENSAJE_CONFIRMAR_PEDIDO, tipo_mensaje);
	}
	return false;
}

bool validar_argumentos(char* tipo_mensaje, char** mensaje_completo){

	int cant_argumentos_mensaje = cantidad_argumentos(mensaje_completo);

	struct_code tipo_estructura = op_code_to_struct_code(string_to_op_code(tipo_mensaje));

	char** formato;
	int cant_argumentos_formato = 0;

	switch(tipo_estructura){
	case STRC_MENSAJE_VACIO: return cant_argumentos_mensaje == 0;

	case STRC_SELECCIONAR_RESTAURANTE:
		formato = string_split(FORMATO_SELECCIONAR_RESTAURANTE, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato;

	case STRC_ID_CONFIRMACION:
		formato = string_split(FORMATO_CONFIRMAR_PEDIDO, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato; //como cuenta la cantidad de arg no importa cual mensaje sea

	case STRC_NOMBRE:
		formato = string_split(FORMATO_CONSULTAR_PLATOS, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato;

	case STRC_NOMBRE_ID:
		formato = string_split(FORMATO_GUARDAR_PEDIDO, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato;

	case STRC_PLATO_LISTO:
		formato = string_split(FORMATO_PLATO_LISTO, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato;

	case STRC_GUARDAR_PLATO:
		formato = string_split(FORMATO_GUARDAR_PLATO, " ");
		cant_argumentos_formato = cantidad_argumentos(formato);
		liberar_vector(formato);
		return cant_argumentos_mensaje == cant_argumentos_formato;

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


void conexionEnvio(){
	int socket = iniciar_cliente(conexion->ip, conexion->puerto);

	if(socket != -1){

		conexion_ok = true;
		pthread_mutex_unlock(&iniciar_consola_mtx);

		if(string_equals_ignore_case(proceso, APP)){
			t_mensaje* handshake_app = malloc(sizeof(t_mensaje));
			handshake_app->tipo_mensaje = POSICION_CLIENTE;
			t_cliente* cliente = malloc(sizeof(t_cliente));

			cliente->id = config_get_int_value(config_cliente, ID_CLIENTE);
			cliente->posicion.x = config_get_int_value(config_cliente, POSICION_X);
			cliente->posicion.y = config_get_int_value(config_cliente, POSICION_Y);

			handshake_app->parametros = cliente;

			enviar_mensaje(handshake_app, socket);
//			free_struct_mensaje(handshake_app->parametros, handshake_app->tipo_mensaje);
			//			loggear_mensaje_enviado(handshake_app->parametros, handshake_app->tipo_mensaje, log_cliente);
		}

		while(1){ //buscar condicion de que siga ejecutando
			sem_wait(&sem_mensajes_a_enviar);
			t_mensaje* mensaje = queue_pop(mensajes_a_enviar);
			enviar_mensaje(mensaje, socket);
			loggear_mensaje_enviado(mensaje->parametros, mensaje->tipo_mensaje, log_cliente);
			free_struct_mensaje(mensaje->parametros, mensaje->tipo_mensaje);
			free(mensaje);

		}
	}else{
		conexion_ok = false;
		pthread_mutex_unlock(&iniciar_consola_mtx);
	}
}

void conexionRecepcion(){

	int socket_servidor = iniciar_cliente(conexion->ip,conexion->puerto);

	int size = 0;
	op_code cod_op;
	int _recv;
	while(1){
		_recv = recv(socket_servidor, &cod_op, sizeof(op_code), MSG_WAITALL);

		if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO && _recv != 0){
			void* buffer = recibir_mensaje(socket_servidor, &size);
			void* mensaje = deserializar_mensaje(buffer, cod_op);
			loggear_mensaje_recibido(mensaje, cod_op, log_cliente);
			free_struct_mensaje(mensaje, cod_op);
			free(buffer);
		}else{
			loggear_mensaje_recibido(NULL, cod_op, log_cliente);
		}
	}

	liberar_conexion(socket_servidor);
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
	*numero = atoi(parametros[1]);

	mensaje->parametros = numero;

	liberar_vector(parametros);
	return mensaje;
}

t_mensaje* llenar_nombre_y_id(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	t_nombre_y_id* nombre_id = malloc(sizeof(t_nombre_y_id));
	nombre_id->nombre.nombre = string_duplicate(parametros[1]);
	nombre_id->id = atoi(parametros[2]);
	mensaje->parametros = nombre_id;
	liberar_vector(parametros);
	return mensaje;
}

t_mensaje* llenar_nombre(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	t_nombre* nombre = malloc(sizeof(t_nombre));
	nombre->nombre = string_duplicate(parametros[1]);
	mensaje->parametros = nombre;
	liberar_vector(parametros);
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
	liberar_vector(parametros);
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
	liberar_vector(parametros);
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
	liberar_vector(parametros);
	return mensaje;
}

t_mensaje* llenar_seleccionar_restaurante(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	m_seleccionarRestaurante* seleccionarRestaurante = malloc(sizeof(m_seleccionarRestaurante));
	seleccionarRestaurante->restaurante.nombre = string_duplicate(parametros[1]);
	seleccionarRestaurante->cliente = atoi(parametros[2]);
	mensaje->parametros = seleccionarRestaurante;
	liberar_vector(parametros);
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
	liberar_vector(parametros);
	return mensaje;
}


t_mensaje* llenar_vacio(char** parametros){
	t_mensaje* mensaje = malloc(sizeof(t_mensaje));
	mensaje->tipo_mensaje = string_to_op_code(parametros[0]);
	liberar_vector(parametros);
	return mensaje;
}
