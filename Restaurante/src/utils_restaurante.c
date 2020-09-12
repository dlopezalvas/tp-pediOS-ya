/*
 * utils_restaurante.c
 *
 *  Created on: 1 sep. 2020
 *      Author: utnso
 */

#include "utils_restaurante.h"


void cargar_configuracion(){

	log_info(log_config_ini, "Cargar configuracion \n");



	cfg_puerto_sindicato=config_get_int_value(config,"PUERTO_SINDICATO");
	log_info(log_config_ini, "\t\tcfg_puerto_sindicato: %d \n",cfg_puerto_sindicato);


	cfg_ip_sindicato=config_get_string_value(config,"IP_SINDICATO");
	log_info(log_config_ini, "\t\tcfg_ip_sindicato: %s \n",cfg_ip_sindicato);


	cfg_puerto_escucha=config_get_int_value(config,"PUERTO_ESCUCHA");
	log_info(log_config_ini, "\t\tcfg_puerto_escucha: %d \n",cfg_puerto_escucha);

	cfg_puerto_app=config_get_int_value(config,"PUERTO_APP");
	log_info(log_config_ini, "\t\tcfg_puerto_app: %d \n",cfg_puerto_app);

	cfg_ip_app=config_get_string_value(config,"IP_APP");
	log_info(log_config_ini, "\t\tcfg_ip_app: %s \n",cfg_ip_app);

	cfg_nombre_restaurante=config_get_string_value(config,"NOMBRE_RESTAURANTE");
	log_info(log_config_ini, "\t\tcfg_nombre_restaurante: %s \n",cfg_nombre_restaurante);

	cfg_algoritmo_planificacion=config_get_string_value(config,"ALGORITMO_PLANIFICACION");
	log_info(log_config_ini, "\t\tcfg_algoritmo_planificacion: %s \n",cfg_algoritmo_planificacion);

	cfg_quantum=config_get_int_value(config,"QUANTUM");
	log_info(log_config_ini, "\t\tcfg_quantum: %d \n",cfg_quantum);
	hilos = list_create();

}




void iniciar_restaurante(){
	log_info(log_config_ini, "Iniciar restaurante \n");

	int estado_conexion;

//CONEXION SINDICATO

	estado_conexion= conectar_con_sindicato();


	//GET DE METADATA

	if(estado_conexion==1){
		//GET DE METADATA A SINDICATO

		//Enviar mje obtener restaurante

		t_mensaje* tipomje = malloc(sizeof(t_mensaje));
		tipomje->tipo_mensaje=OBTENER_RESTAURANTE;

		t_nombre* nombre_restaurante=malloc(sizeof(t_nombre));
		nombre_restaurante->nombre="uggis";

		tipomje->parametros=nombre_restaurante;

		//enviar_mensaje(tipomje, conexion_sindicato);

		//Recibir respuesta obtener restaurante
		metadata_rest = malloc(sizeof(rta_obtenerRestaurante));

		//metadata_rest = metadata_restaurante(conexion_sindicato);
	}else{

		log_info(log_config_ini, "\tSe cargan los datos de default, Sindicato no sisponible \n");

		metadata_rest = malloc(sizeof(rta_obtenerRestaurante));

		//obtengo los datos del restaurante de un archivo para hacer pruebas
		config_sindicato = leer_config("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Restaurante/src/default_sindicato.config");
		int cantidad_cocineros=config_get_int_value(config_sindicato,"CANTIDAD_COCINEROS");
		char* afinidad1 =config_get_string_value(config_sindicato,"AFINIDAD_COCINEROS1");
		char* afinidad2 =config_get_string_value(config_sindicato,"AFINIDAD_COCINEROS2");
		int posx =config_get_int_value(config_sindicato,"POSX");
		int posy =config_get_int_value(config_sindicato,"POSY");
		int cant_recetas=config_get_int_value(config_sindicato,"CANTRECETAS");
		char* receta1 =config_get_string_value(config_sindicato,"RECETA1");
		char* receta2 =config_get_string_value(config_sindicato,"RECETA2");
		char* receta3 =config_get_string_value(config_sindicato,"RECETA3");
		int hornos= config_get_int_value(config_sindicato,"CANTIDAD_HORNOS");

		log_info(log_config_ini, "\tTermino la lectura del archivo de datos default \n");


		//Cargo los datos de prueba

		metadata_rest->cantCocineroAfinidad=cantidad_cocineros;
		log_info(log_config_ini, "\t\tcantidad de cocineros %d \n",metadata_rest->cantCocineroAfinidad);

		metadata_rest->cocineroAfinidad = list_create();
		list_add(metadata_rest->cocineroAfinidad, afinidad1);
		char* afi1= list_get(metadata_rest->cocineroAfinidad, 0);
		log_info(log_config_ini, "\t\tafinidad1: %s \n",afi1);

		list_add(metadata_rest->cocineroAfinidad, afinidad2);
		char* afi2= list_get(metadata_rest->cocineroAfinidad, 1);
		log_info(log_config_ini, "\t\tafinidad2: %s \n",afi2);

		metadata_rest->posicion.x=posx;
		metadata_rest->posicion.y=posy;
		log_info(log_config_ini, "\t\tcoordenadas x:%d ,  y:%d \n",metadata_rest->posicion.x,metadata_rest->posicion.y);


		metadata_rest->cantRecetas=cant_recetas;
		log_info(log_config_ini, "\t\tcantiad de recetas: %d \n",metadata_rest->cantRecetas);


		metadata_rest->recetas = list_create();
		list_add(metadata_rest->recetas, receta1);
		log_info(log_config_ini, "\t\treceta1: %s \n",list_get(metadata_rest->recetas, 0));

		list_add(metadata_rest->recetas, receta2);

		log_info(log_config_ini, "\t\treceta2: %s \n",list_get(metadata_rest->recetas, 1));

		list_add(metadata_rest->recetas, receta3);

		log_info(log_config_ini, "\t\treceta3: %s \n",list_get(metadata_rest->recetas, 2));

		metadata_rest->cantHornos=hornos;
		log_info(log_config_ini, "\t\tcantidad de hornos %d  \n", metadata_rest->cantHornos);

		log_info(log_config_ini, "\tDatos default restaurante cargados \n");


		}



	//CREO LAS DISTINTAS COLAS DE READY Y DE ENTRADA SALIDA
		log_info(log_config_ini, "\tIniciar_colas_ready_es \n");
		//iniciar_colas_ready_es (rta_obtenerRestaurante* metadata);

	//CREAR PROCESO PLANIFICADOR
		log_info(log_config_ini, "\tIniciar_planificador de platos \n");
		//pthread_create(plafinificar);


}


void* fhilo_servidor_clientes(void* v){

	int conexion_servidor;
	conexion_servidor = iniciar_servidor(cfg_puerto_escucha);
	log_info(log_config_ini, "El servidor se inicio correctamente en : %d \n",conexion_servidor);
	    while(1) {
	        esperar_cliente(conexion_servidor);

	    }

}


void esperar_cliente(int servidor){

	struct sockaddr_in direccion_cliente;

	unsigned int tam_direccion = sizeof(struct sockaddr_in);

	int cliente = accept (servidor, (void*) &direccion_cliente, &tam_direccion);
	pthread_t hilo;

	pthread_mutex_lock(&mutex_hilos);
	list_add(hilos, &hilo);
	pthread_mutex_unlock(&mutex_hilos);

	pthread_create(&hilo,NULL,(void*)serve_client,cliente);
	pthread_detach(hilo);

}


void serve_client(int socket){
	int rec;
	int cod_op;
	while(1){
		rec = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if(rec == -1 || rec == 0 ){
			cod_op = -1;
			//			pthread_mutex_lock(&logger_mutex);
			//			log_info(logger,"Se desconecto el proceso con id: %d",socket);
			//			pthread_mutex_unlock(&logger_mutex);
			pthread_exit(NULL);
		}
		puts("recibi un mensaje");
		printf("codigo: %d\n", cod_op);
		process_request(cod_op, socket);
	}
}

void process_request(int cod_op, int cliente_fd) {
	int size = 0;
	void* buffer = recibir_mensaje(cliente_fd, &size);

    //void* mensaje_deserializado = deserialilzar_mensaje(buffer, cod_op);



    // TODO: switch con tipos de mensaje
	switch (cod_op) {
		case CONSULTAR_PLATOS:

		log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PLATOS: ",cod_op);

		//DESERIALIZO EL MJ


		//ENVIAR MJ A SINDICATO

		//RECIBO RESPUESTA

		//RESPONDO AL CLIENTE



			//free(buffer);
			break;
		case CREAR_PEDIDO:

		log_info(log_config_ini ,"Se recibio el mj CREAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ


		//ENVIAR MJ A SINDICATO

		//RECIBO RESPUESTA

		//RESPONDO AL CLIENTE

		break;
		case AGREGAR_PLATO:
		log_info(log_config_ini ,"Se recibio el mj AGREGAR_PLATO: ",cod_op);

		//DESERIALIZO EL MJ


		//ENVIAR MJ A SINDICATO

		//RECIBO RESPUESTA

		//RESPONDO AL CLIENTE

		break;

		case CONFIRMAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONFIRMAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ

		/*
		Este mensaje permitirá confirmar un pedido creado previamente. Para esto se recibirá el ID de pedido destino.
		La funcionalidad de este mensaje se descompone en los siguientes pasos:
		Obtener el Pedido desde el Módulo Sindicato.
		Generar el PCB (Plato Control Block) de cada plato del pedido en cuestión y dejarlo en el ciclo de planificación
		(que se explicará a continuación en el módulo de Planificación de Platos).
		Para esto, se deberá ejecutar el mensaje Obtener Receta al Módulo Sindicato para saber la trazabilidad que deberá tener al momento de su ejecución.
		Cabe aclarar que el número de pedido se deberá guardar dentro del PCB para su futuro uso.
		Informar al Módulo que lo invocó que su pedido fue confirmado.
		 */


		//LE AVISO AL HILO PLANIFICADOR QUE TIENE UN PEDIDO CON PLATOS A PLANIFICAR
			//plafinificar

		//RESPONDO AL CLIENTE
			//OK o FAIL

		break;
		case CONSULTAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ


		//ENVIAR MJ A SINDICATO

		//RECIBO RESPUESTA

		//RESPONDO AL CLIENTE
		break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}



}






void delay (int number_of_seconds){
	int milli_seconds = 1000000* number_of_seconds;
	clock_t start_time = clock();
	while (clock() < start_time + milli_seconds);
}

rta_obtenerRestaurante* metadata_restaurante(int socket){

	int cod_op;
	if(recv(socket, &cod_op, sizeof(int), MSG_WAITALL)==1);
	cod_op=-1;

	int size=0;
	void* stream = recibir_mensaje(socket,&size);

	rta_obtenerRestaurante* restaurante =malloc(sizeof(rta_obtenerRestaurante));
	//restaurante = deserializar_mensaje(stream); //dudas

return restaurante;
}
int  conectar_con_sindicato(){

	int estado_conexion=-1;
	int conexion_sindicato;


	log_info(log_config_ini, "\tInicia intento de conexion con Sindicato \n");


	conexion_sindicato = iniciar_cliente(cfg_ip_sindicato,cfg_puerto_sindicato);


	estado_conexion= conexion_sindicato;
	if(estado_conexion==-1){

		log_info(log_config_ini, "\tModulo sindicato no disponible \n");


	}else{

		  log_info(log_config_ini,"\tLa reconexión con Sindicato fue exitosa \n");
		  estado_conexion=1;
		}
return estado_conexion;

}

