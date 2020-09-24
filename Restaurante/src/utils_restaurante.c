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

	//INICIO DE LISTAS GLOBALES

	inicio_de_listas_globales();



	//CREO LAS DISTINTAS COLAS DE READY Y DE ENTRADA SALIDA
		log_info(log_config_ini, "\tIniciar_colas_ready_es \n");
		iniciar_colas_ready_es (metadata_rest);

	//CREAR PROCESO PLANIFICADOR
		log_info(log_config_ini, "Iniciar_planificador de platos \n");
		//pthread_create(plafinificar);
		pthread_create(&hilo_servidor_clientes, NULL,(void*) fhilo_servidor_clientes, NULL);
		pthread_create(&hilo_planificador, NULL,(void*) fhilo_planificador, NULL);




}


void inicio_de_listas_globales(){

//LSITA DE HILOS CLIENTE
	hilos = list_create();
	status_platos = list_create();


}

void iniciar_colas_ready_es(rta_obtenerRestaurante* metadata){




	log_info(log_config_ini, "Comienza proceso de colas");



	log_info(log_config_ini, "\treceta1: %s \n",list_get(metadata->recetas, 0));



	log_info(log_config_ini, "\treceta2: %s \n",list_get(metadata->recetas, 1));



	log_info(log_config_ini, "\treceta3: %s \n",list_get(metadata->recetas, 2));


	log_info(log_config_ini, "\tcantidad de hornos %d  \n", metadata->cantHornos);

	log_info(log_config_ini, "Fin inicializacion colas de ready \n");
}

void* fhilo_planificador(void* v){
	int contador=0;

	while (contador <10){

		log_info(log_config_ini, "\tCONTADOR: %d \n",contador);

		delay(3);
		contador=contador+1;

	}

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

			pthread_exit(NULL);
		}
		log_info(log_config_ini ,"Recibi un mj \n");
		log_info(log_config_ini ,"Se recibio cod_op: %d \n",cod_op);

		process_request(cod_op, socket);
	}
}

void process_request(int cod_op, int cliente_fd) {
	int size = 0;
	    void* mensaje = NULL;
	    if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
	        void* buffer = recibir_mensaje(cliente_fd, &size);
	        mensaje = deserializar_mensaje(buffer, cod_op);
	    }




    // TODO: switch con tipos de mensaje
	switch (cod_op) {
		case CONSULTAR_PLATOS:

			log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PLATOS: ",cod_op);

			//DESERIALIZO EL MJ (no hace falta)
			//if(pregunto si el nombre de restaurante es igual al mio)
			//t_nombre* nombre = malloc(sizeof(t_nombre));
			//nombre= mensaje;

			//CARGO EL MJ
			t_mensaje * sindicato_CONSULTAR_PLATOS = malloc(sizeof(t_mensaje));
			sindicato_CONSULTAR_PLATOS->tipo_mensaje=CONSULTAR_PLATOS;
			sindicato_CONSULTAR_PLATOS->parametros=mensaje;



			//CONECTARME CON SINDICATO y MANDARLE EL MJ, RECIBIR RTA Y RESPONDER AL CLIENTE

			int socket_CONSULTAR_PLATOS = conectar_con_sindicato();

			t_mensaje * cliente_RTA_CONSULTAR_PLATOS = malloc(sizeof(t_mensaje));

			if(socket_CONSULTAR_PLATOS==-1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

			//enviar al cliente un codigo de error
			cliente_RTA_CONSULTAR_PLATOS=NULL;
			enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);

			}else{

			//ENVIO A SINDICATO
			enviar_mensaje(sindicato_CONSULTAR_PLATOS,socket_CONSULTAR_PLATOS);

			//RECIBO RESPUESTA DE SINDICATO
			t_restaurante_y_plato* rta_sindicato_CONSULTAR_PLATOS= malloc(sizeof(t_restaurante_y_plato));

			rta_sindicato_CONSULTAR_PLATOS= recibir_RTA_CONSULTAR_PLATOS(socket_CONSULTAR_PLATOS);


			//ENVIAR RESPUESTA DE SINDICATO A CLIENTE

			cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje=RTA_CONSULTAR_PLATOS;
			cliente_RTA_CONSULTAR_PLATOS->parametros=rta_sindicato_CONSULTAR_PLATOS;


			enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);

			log_info(log_config_ini,"\tSe envio el mj al cliente\n");

			}


			//free(buffer);
			break;
		case CREAR_PEDIDO:

			log_info(log_config_ini ,"Se recibio el mj CREAR_PEDIDO: ",cod_op);

			//esta es la respuesta al cliente, pero el mj responde el id de pedido
			//uint32_t* ok = malloc(sizeof(uint32_t));
			//*ok = 1; //1 para OK, 0 para FAIL

			//CREAR ID PEDIDO
			pthread_mutex_lock(&mutex_id_pedidos);
			id_pedidos=id_pedidos+1;
			pthread_mutex_unlock(&mutex_id_pedidos);


			uint32_t* id_pedido = malloc(sizeof(uint32_t));
			id_pedido=id_pedidos;

			t_mensaje* cliente_rta_CREAR_PEDIDO= malloc(sizeof(t_mensaje));

			//ENVIAR EL ID A SINDICADO PARA GUARDAR PEDIDO

			t_nombre_y_id* mje_GUARDAR_PEDIDO=malloc(sizeof(t_nombre_y_id));
			mje_GUARDAR_PEDIDO->id=id_pedido;
			mje_GUARDAR_PEDIDO->nombre=cfg_nombre_restaurante;

			t_mensaje* rta_GUARDAR_PEDIDO= malloc(sizeof(t_mensaje));
			rta_GUARDAR_PEDIDO->tipo_mensaje=GUARDAR_PEDIDO;
			rta_GUARDAR_PEDIDO->parametros=mje_GUARDAR_PEDIDO;

			int socket_GUARDAR_PEDIDO = conectar_con_sindicato();
			if(socket_GUARDAR_PEDIDO==-1){

				log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

				//enviar al cliente un codigo de error
				cliente_rta_CREAR_PEDIDO=NULL;
				enviar_mensaje(cliente_rta_CREAR_PEDIDO,cliente_fd);

			}else{
				enviar_mensaje(rta_GUARDAR_PEDIDO,socket_GUARDAR_PEDIDO);


				//RECIBIR RESPUESTA DE SINDICATO
				uint32_t rta_sindicato_GUARDAR_PEDIDO= malloc(sizeof(uint32_t));

				rta_sindicato_GUARDAR_PEDIDO= recibir_RTA_GUARDAR_PEDIDO(socket_CONSULTAR_PLATOS);// TODO

				//recibo un ok/fail pero al cliente le envio el id del pedido creado
				//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO

				cliente_rta_CREAR_PEDIDO->tipo_mensaje=RTA_CREAR_PEDIDO;
				cliente_rta_CREAR_PEDIDO->parametros=id_pedido;

				enviar_mensaje(cliente_rta_CREAR_PEDIDO, cliente_fd);
				log_info(log_config_ini ,"Se envio: RTA_CREAR_PEDIDO ",cod_op);

			}
		break;
		case AGREGAR_PLATO:
		log_info(log_config_ini ,"Se recibio el mj AGREGAR_PLATO: ",cod_op);


		//A través del envío del mensaje Guardar Plato al Módulo Sindicato, agrega un plato correspondiente a un pedido específico, que se encontrará relacionado con el
		//Restaurante que envió dicho mensaje. Solo se podrá crear platos sobre pedidos existentes.

		//DESERIALIZO EL MJ

		t_nombre_y_id* mj_agregar_plato = malloc(sizeof(t_nombre_y_id));
		mj_agregar_plato = mensaje;

		t_mensaje * sindicato_GUARDAR_PLATO = malloc(sizeof(t_mensaje));
		sindicato_GUARDAR_PLATO->tipo_mensaje=GUARDAR_PLATO;
		sindicato_GUARDAR_PLATO->parametros=mj_agregar_plato;

		t_mensaje* cliente_rta_AGREGAR_PLATO= malloc(sizeof(t_mensaje));

		//VALIDACION DE ID PEDIDO Y RESTAURANTE
		if(mj_agregar_plato->id<=id_pedidos){


			t_nombre* nombre_restaurante_GUARDAR_PLATO = malloc(sizeof(t_nombre));
			nombre_restaurante_GUARDAR_PLATO=cfg_nombre_restaurante;
			nombre_restaurante_GUARDAR_PLATO=length(cfg_nombre_restaurante);

			t_nombre* nombre_plato_GUARDAR_PLATO = malloc(sizeof(t_nombre));
			nombre_plato_GUARDAR_PLATO=mj_agregar_plato->nombre;
			nombre_plato_GUARDAR_PLATO=length(mj_agregar_plato->nombre);

			m_guardarPlato* sindicato_GUARDAR_PLATO = malloc(sizeof(m_guardarPlato));
			sindicato_GUARDAR_PLATO->cantidad=1;
			sindicato_GUARDAR_PLATO->comida=nombre_plato_GUARDAR_PLATO;
			sindicato_GUARDAR_PLATO->idPedido=mj_agregar_plato->id;
			sindicato_GUARDAR_PLATO->restaurante=nombre_restaurante_GUARDAR_PLATO;

			t_mensaje* mje_sindicato_GUARDAR_PLATO= malloc(sizeof(t_mensaje));
			mje_sindicato_GUARDAR_PLATO->tipo_mensaje=GUARDAR_PLATO;
			mje_sindicato_GUARDAR_PLATO->parametros=sindicato_GUARDAR_PLATO;

			//CONEXION CON SINDICATO
			int socket_GUARDAR_PLATO = conectar_con_sindicato();

			if(socket_GUARDAR_PLATO==-1){

				log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

				//enviar al cliente un codigo de error
				cliente_rta_AGREGAR_PLATO=NULL;
				enviar_mensaje(cliente_rta_AGREGAR_PLATO,cliente_fd);

			}else{

				enviar_mensaje(mje_sindicato_GUARDAR_PLATO,socket_GUARDAR_PLATO);

				//RECIBIR RESPUESTA DE SINDICATO
				uint32_t rta_sindicato_GUARDAR_PLATO= malloc(sizeof(uint32_t));

				rta_sindicato_GUARDAR_PLATO= recibir_RTA_GUARDAR_PLATO(socket_GUARDAR_PLATO);// TODO

				//recibo un ok/fail y al cliente le mando el mismo ok/fail
				//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO

				cliente_rta_AGREGAR_PLATO->tipo_mensaje=RTA_AGREGAR_PLATO;
				cliente_rta_AGREGAR_PLATO->parametros=rta_sindicato_GUARDAR_PLATO;

				enviar_mensaje(cliente_rta_AGREGAR_PLATO, cliente_fd);
				log_info(log_config_ini ,"Se envio: RTA_AGREGAR_PLATO ",cod_op);
			}

		}else{


			log_info(log_config_ini ,"El id de pedido no corresponde a este restaurante");
		}






		break;

		case CONFIRMAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONFIRMAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ
		int32_t* id_CONFIRMAR_PEDIDO = malloc(sizeof(uint32_t));

		id_CONFIRMAR_PEDIDO=mensaje;

		//1) OBTENER EL PEDIDO DEL MODULO SINDICATO
		t_nombre* nombre_restaurante_OBTENER_PEDIDO = malloc(sizeof(t_nombre));
		nombre_restaurante_OBTENER_PEDIDO=cfg_nombre_restaurante;
		nombre_restaurante_OBTENER_PEDIDO=length(cfg_nombre_restaurante);


		t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
		sindicato_OBTENER_PEDIDO->id=id_CONFIRMAR_PEDIDO;
		sindicato_OBTENER_PEDIDO->nombre=nombre_restaurante_OBTENER_PEDIDO;


		t_mensaje* mje_sindicato_OBTENER_PEDIDO= malloc(sizeof(t_mensaje));
		mje_sindicato_OBTENER_PEDIDO->tipo_mensaje=OBTENER_PEDIDO;
		mje_sindicato_OBTENER_PEDIDO->parametros=sindicato_OBTENER_PEDIDO;

		//estructura de respuesta al cliente
		t_mensaje* cliente_rta_CONFIRMAR_PEDIDO= malloc(sizeof(t_mensaje));

			//CONEXION CON SINDICATO


			int socket_OBTENER_PEDIDO = conectar_con_sindicato();

			if(socket_OBTENER_PEDIDO==-1){
				log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

				//enviar al cliente un codigo de error
				cliente_rta_AGREGAR_PLATO=NULL;
				enviar_mensaje(cliente_rta_AGREGAR_PLATO,cliente_fd);

			}else{
					enviar_mensaje(mje_sindicato_OBTENER_PEDIDO,socket_OBTENER_PEDIDO);

					//RECIBIR RESPUESTA DE SINDICATO
					rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO= malloc(sizeof(uint32_t));

					rta_sindicato_RTA_OBTENER_PEDIDO= recibir_RTA_OBTENER_PEDIDO(socket_OBTENER_PEDIDO);// TODO
					/*
					typedef struct{
						t_nombre comida;
						uint32_t cantTotal;
						uint32_t cantHecha;
					}t_elemPedido;

					typedef struct{
						uint32_t cantPedidos;
						t_list* infoPedidos; //lista de elemPedido
					}rta_obtenerPedido;
				*/
			}
		//2) GENERAR EL PCB DE CADA PLATO DED PEDIDO


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


	int conexion_sindicato;


	log_info(log_config_ini, "\tInicia intento de conexion con Sindicato \n");


	conexion_sindicato = iniciar_cliente(cfg_ip_sindicato,cfg_puerto_sindicato);



	if(conexion_sindicato==-1){

		log_info(log_config_ini, "\tModulo sindicato no disponible \n");


	}else{

		  log_info(log_config_ini,"\tLa reconexión con Sindicato fue exitosa \n");

		}
return conexion_sindicato;

}
t_restaurante_y_plato*  recibir_RTA_CONSULTAR_PLATOS(int socket){
		int rec;
		int cod_op;
		t_restaurante_y_plato* mensaje = NULL;

			rec = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
			if(rec == -1 || rec == 0 ){
				cod_op = -1;

			}

			if(cod_op==-1) {

				//enviar error

			}else{
				int size = 0;


				if(op_code_to_struct_code(cod_op) != STRC_MENSAJE_VACIO){
					void* buffer = recibir_mensaje(socket, &size);
					mensaje = deserializar_mensaje(buffer, cod_op);
				}

			}
return mensaje;

}
