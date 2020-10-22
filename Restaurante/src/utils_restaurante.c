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

	int conexion_sindicato;

//CONEXION SINDICATO

	conexion_sindicato= conectar_con_sindicato();


	//GET DE METADATA
	metadata_rest = malloc(sizeof(rta_obtenerRestaurante));

	if(conexion_sindicato != -1){
		//GET DE METADATA A SINDICATO

		//Enviar mje obtener restaurante


		t_mensaje* tipomje = malloc(sizeof(t_mensaje));
		tipomje->tipo_mensaje=OBTENER_RESTAURANTE;


		t_nombre* nombre_restaurante=malloc(sizeof(t_nombre));
		nombre_restaurante->nombre="uggis";
		nombre_restaurante->largo_nombre=strlen(cfg_nombre_restaurante);

		tipomje->parametros=nombre_restaurante;

		enviar_mensaje(tipomje, conexion_sindicato);

		//Recibir respuesta obtener restaurante


		metadata_rest = metadata_restaurante(conexion_sindicato);
	}else{

		log_info(log_config_ini, "\tSe cargan los datos de default, Sindicato no sisponible \n");


		printf("paso la creacion de var");

		//obtengo los datos del restaurante de un archivo para hacer pruebas
		config_sindicato = leer_config("/home/utnso/workspace/tp-2020-2c-CoronaLinux/Restaurante/src/default_sindicato.config");
		int cantidad_cocineros=config_get_int_value(config_sindicato,"CANTIDAD_COCINEROS");
		char* afinidad1 =config_get_string_value(config_sindicato,"AFINIDAD_COCINEROS1");
		char* afinidad2 =config_get_string_value(config_sindicato,"AFINIDAD_COCINEROS2");
		int posx =config_get_int_value(config_sindicato,"POSX");
		int posy =config_get_int_value(config_sindicato,"POSY");
		int cant_recetas=config_get_int_value(config_sindicato,"CANTRECETAS");



		t_nombre* nom_receta1=malloc(sizeof(t_mensaje));
		nom_receta1->largo_nombre=strlen(config_get_string_value(config_sindicato,"RECETA1"));
		nom_receta1->nombre=config_get_string_value(config_sindicato,"RECETA1");
		t_receta* receta1=malloc(sizeof(t_mensaje));
		receta1->receta.nombre=nom_receta1;

		t_nombre* nom_receta2=malloc(sizeof(t_mensaje));
		nom_receta2->largo_nombre=strlen(config_get_string_value(config_sindicato,"RECETA2"));
		nom_receta2->nombre =config_get_string_value(config_sindicato,"RECETA2");
		t_receta* receta2=malloc(sizeof(t_mensaje));
		receta2->receta.nombre=nom_receta2;

		t_nombre* nom_receta3=malloc(sizeof(t_mensaje));
		nom_receta3->largo_nombre=strlen(config_get_string_value(config_sindicato,"RECETA3"));
		nom_receta3->nombre =config_get_string_value(config_sindicato,"RECETA3");
		t_receta* receta3=malloc(sizeof(t_mensaje));
		receta3->receta.nombre=nom_receta3;

		int hornos= config_get_int_value(config_sindicato,"CANTIDAD_HORNOS");

		log_info(log_config_ini, "\tTermino la lectura del archivo de datos default \n");






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

/*
		list_add(metadata_rest->recetas, receta1);
		log_info(log_config_ini, "\t\treceta1: %s \n",list_get(metadata_rest->recetas, 0));

		list_add(metadata_rest->recetas, receta2);

		log_info(log_config_ini, "\t\treceta2: %s \n",list_get(metadata_rest->recetas, 1));

		list_add(metadata_rest->recetas, receta3);

		log_info(log_config_ini, "\t\treceta3: %s \n",list_get(metadata_rest->recetas, 2));

		metadata_rest->cantHornos=hornos;
		log_info(log_config_ini, "\t\tcantidad de hornos %d  \n", metadata_rest->cantHornos);
*/
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

/*
 * Creación de colas de planificación
En base a los Hornos y los cocineros con sus afinidades que se obtuvieron, se deberán crear las distintas colas de ready y de entrada salida.
De esta manera, si tengo dos cocineros que tienen como afinidad a Milanesa se deberá crear dos colas de Ready en las cuales todas las milanesas irán a dicha cola y los demás platos irán a otra. Un cocinero con afinidades solo podrá cocinar aquellas recetas a las cuales es afín.
Por otro lado, durante la ejecución de un plato puede darse que se requiera enviarlo al horno, en estos casos existirá una cantidad de Hornos finitos dentro de cada restaurante que funcionarán como entradas salidas visto en la teoría.
 *
 */


log_info(log_config_ini, "Comienza proceso de colas");

uint32_t cant_cocineros=metadata->cantCocineroAfinidad;
log_info(log_config_ini, "\tcant_cocineros: %d \n",cant_cocineros);


int cant_afinidades= metadata->cocineroAfinidad->elements_count;
log_info(log_config_ini, "\tcant_afinidades: %d \n",cant_afinidades);

t_receta* metadata_receta1 = malloc(sizeof(t_receta));
printf("seg fault");
metadata_receta1=list_get(metadata->recetas, 0);

log_info(log_config_ini, "\treceta1: %s \n",metadata_receta1->receta);

t_receta* metadata_receta2 = malloc(sizeof(t_receta));
metadata_receta2=list_get(metadata->recetas, 1);
log_info(log_config_ini, "\treceta2 %s \n",metadata_receta2->receta);

uint32_t cant_hornos= metadata->cantHornos;
log_info(log_config_ini, "\tcantidad de hornos %d  \n", cant_hornos);


id_pedidos=5;
log_info(log_config_ini, "\tcantidad de pedidos %d  \n", id_pedidos);
//Logica de generacion de colas de ready y i/o
//TODO




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
			mje_GUARDAR_PEDIDO->nombre.nombre=cfg_nombre_restaurante;
			mje_GUARDAR_PEDIDO->nombre.largo_nombre=cfg_nombre_restaurante;

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
			nombre_restaurante_GUARDAR_PLATO->nombre=cfg_nombre_restaurante;
			nombre_restaurante_GUARDAR_PLATO->largo_nombre=strlen(cfg_nombre_restaurante);

			t_nombre* nombre_plato_GUARDAR_PLATO = malloc(sizeof(t_nombre));
			nombre_plato_GUARDAR_PLATO->nombre=mj_agregar_plato->nombre.nombre;
			nombre_plato_GUARDAR_PLATO=mj_agregar_plato->nombre.largo_nombre;

			m_guardarPlato* sindicato_GUARDAR_PLATO = malloc(sizeof(m_guardarPlato));
			sindicato_GUARDAR_PLATO->cantidad=1;
			sindicato_GUARDAR_PLATO->comida.largo_nombre=nombre_plato_GUARDAR_PLATO->largo_nombre;
			sindicato_GUARDAR_PLATO->comida.nombre=nombre_plato_GUARDAR_PLATO->nombre;
			sindicato_GUARDAR_PLATO->idPedido=mj_agregar_plato->id;
			sindicato_GUARDAR_PLATO->restaurante.largo_nombre=nombre_restaurante_GUARDAR_PLATO->largo_nombre;
			sindicato_GUARDAR_PLATO->restaurante.nombre=nombre_restaurante_GUARDAR_PLATO->nombre;

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
		nombre_restaurante_OBTENER_PEDIDO->nombre=cfg_nombre_restaurante;
		nombre_restaurante_OBTENER_PEDIDO->largo_nombre=strlen(cfg_nombre_restaurante);


		t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
		sindicato_OBTENER_PEDIDO->id=id_CONFIRMAR_PEDIDO;
		sindicato_OBTENER_PEDIDO->nombre.largo_nombre=nombre_restaurante_OBTENER_PEDIDO->largo_nombre;
		sindicato_OBTENER_PEDIDO->nombre.nombre=nombre_restaurante_OBTENER_PEDIDO->nombre;


		t_mensaje* mje_sindicato_OBTENER_PEDIDO= malloc(sizeof(t_mensaje));
		mje_sindicato_OBTENER_PEDIDO->tipo_mensaje=OBTENER_PEDIDO;
		mje_sindicato_OBTENER_PEDIDO->parametros=sindicato_OBTENER_PEDIDO;

		//estructura de respuesta al cliente
		t_mensaje* cliente_rta_CONFIRMAR_PEDIDO= malloc(sizeof(t_mensaje));
		rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO= malloc(sizeof(rta_obtenerPedido));

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


					rta_sindicato_RTA_OBTENER_PEDIDO= recibir_RTA_OBTENER_PEDIDO(socket_OBTENER_PEDIDO);// TODO
					liberar_conexion( socket_OBTENER_PEDIDO);

			}

			//2) GENERAR EL PCB DE CADA PLATO DED PEDIDO - OBTENER RECETA

			pthread_t* hilo_plato;
			t_elemPedido* pedido_n = malloc(sizeof(t_elemPedido));
			t_nombre* sindicato_nombre_plato_receta = malloc(sizeof(t_nombre));

			t_mensaje* mje_sindicato_OBTENER_RECETA= malloc(sizeof(t_mensaje));
			mje_sindicato_OBTENER_RECETA->tipo_mensaje=OBTENER_RECETA;
			rta_obtenerReceta* rta_sindicato_RTA_OBTENER_RECETA= malloc(sizeof(rta_obtenerReceta));
			//t_plato_pcb* plato_pcb = malloc(sizeof(t_plato_pcb));//correccion
			t_plato_pcb* plato_pcb;
			//t_nombre* plato_pcb_nombre_plato = malloc(sizeof(t_nombre));//correccion
			t_nombre* plato_pcb_nombre_plato;

			 for (int inicio_plato = 0;
					 inicio_plato <rta_sindicato_RTA_OBTENER_PEDIDO->cantPedidos;
					 inicio_plato++)
			    {
				 plato_pcb = malloc(sizeof(t_plato_pcb));
				 plato_pcb_nombre_plato = malloc(sizeof(t_nombre));

				 pedido_n =  list_get(rta_sindicato_RTA_OBTENER_PEDIDO->infoPedidos, inicio_plato);
				 //obtengo el nombre del plato
				 sindicato_nombre_plato_receta->nombre=pedido_n->comida.nombre;
				 sindicato_nombre_plato_receta->largo_nombre=pedido_n->comida.largo_nombre;

				 //cargo el mj a enviar a sindicato para pedir receta
				 mje_sindicato_OBTENER_PEDIDO->parametros=sindicato_nombre_plato_receta;

				 //conexion sindicato - NO HAGO VALIDACION DE SINDICATO DISPONIBLE
				 int socket_OBTENER_RECETA = conectar_con_sindicato();

				enviar_mensaje(mje_sindicato_OBTENER_RECETA,socket_OBTENER_RECETA);

				//RECIBIR RESPUESTA DE SINDICATO
				rta_sindicato_RTA_OBTENER_RECETA= recibir_RTA_OBTENER_RECETA(socket_OBTENER_RECETA);// TODO
				liberar_conexion( socket_OBTENER_RECETA);


				//cargo el pcb
				plato_pcb->id_pedido=sindicato_OBTENER_PEDIDO->id;
				plato_pcb->comida.nombre=pedido_n->comida.nombre;
				plato_pcb->comida.largo_nombre=pedido_n->comida.largo_nombre;
				plato_pcb->cantTotal=pedido_n->cantTotal;
				plato_pcb->cantHecha=pedido_n->cantHecha;
				plato_pcb->cantPasos=rta_sindicato_RTA_OBTENER_RECETA->cantPasos;
				plato_pcb->pasos=rta_sindicato_RTA_OBTENER_RECETA->pasos;

				//creo hilo pcb

				hilo_plato= malloc(sizeof(pthread_t));

				pthread_mutex_lock(&mutex_pcb);
				list_add(hilos_pcb, hilo_plato);
				pthread_mutex_unlock(&mutex_pcb);

				pthread_create(hilo_plato,NULL,fhilo_plato,plato_pcb);
				pthread_detach(hilo_plato);

			    }
			 //LE AVISO AL HILO PLANIFICADOR QUE TIENE UN PEDIDO CON PLATOS A PLANIFICAR
			 //plafinificar

			//respondo al cliente -- consultar en que casos se debe mandar fail
			//esta es la respuesta al cliente,
			uint32_t* ok = malloc(sizeof(uint32_t));
			*ok = 1; //1 para OK, 0 para FAIL


			cliente_rta_CONFIRMAR_PEDIDO->parametros=ok;
			cliente_rta_CONFIRMAR_PEDIDO->tipo_mensaje=RTA_CONFIRMAR_PEDIDO;
			 enviar_mensaje(cliente_rta_CONFIRMAR_PEDIDO,cliente_fd);



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




		//RESPONDO AL CLIENTE
			//OK o FAIL

		break;
		case CONSULTAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ
		int32_t* id_CONSULTAR_PEDIDO = malloc(sizeof(uint32_t));
		id_CONSULTAR_PEDIDO=mensaje;

		//respuesta cliente

		rta_consultarPedido* rta_CONSULTAR_PEDIDO = malloc(sizeof(rta_consultarPedido));
		t_mensaje* cliente_rta_CONSULTAR_PEDIDO= malloc(sizeof(t_mensaje));


		//ENVIAR MJ A SINDICATO - OBTNER PEDIDO
		//1) OBTENER EL PEDIDO DEL MODULO SINDICATO
				t_nombre* nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO = malloc(sizeof(t_nombre));
				nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->nombre=cfg_nombre_restaurante;
				nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->largo_nombre=strlen(cfg_nombre_restaurante);


				t_nombre_y_id* sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
				sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO->id=id_CONSULTAR_PEDIDO;
				sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO->nombre.nombre=nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->nombre;
				sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO->nombre.largo_nombre=nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->largo_nombre;


				t_mensaje* mje_sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO= malloc(sizeof(t_mensaje));
				mje_sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO->tipo_mensaje=OBTENER_PEDIDO;
				mje_sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO->parametros=sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO;

				//CONEXION CON SINDICATO
				int socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO = conectar_con_sindicato();

				enviar_mensaje(mje_sindicato_CONSULTAR_PEDIDO_OBTENER_PEDIDO,socket_OBTENER_PEDIDO);

				//RECIBIR RESPUESTA DE SINDICATO
				rta_obtenerPedido* rta_sindicato_CONSULTAR_PEDIDO_RTA_OBTENER_PEDIDO= malloc(sizeof(rta_obtenerPedido));
				rta_sindicato_CONSULTAR_PEDIDO_RTA_OBTENER_PEDIDO= recibir_RTA_OBTENER_PEDIDO(socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);// TODO
				liberar_conexion( socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);




		//RECIBO RESPUESTA

		//RESPONDO AL CLIENTE


				rta_CONSULTAR_PEDIDO->restaurante.nombre = nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->nombre;
				rta_CONSULTAR_PEDIDO->restaurante.largo_nombre = nombre_restaurante_CONSULTAR_PEDIDO_OBTENER_PEDIDO->largo_nombre;
				rta_CONSULTAR_PEDIDO->cantPlatos=rta_sindicato_CONSULTAR_PEDIDO_RTA_OBTENER_PEDIDO->cantPedidos;
				rta_CONSULTAR_PEDIDO->platos=rta_sindicato_CONSULTAR_PEDIDO_RTA_OBTENER_PEDIDO->infoPedidos;

				cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje=RTA_CONSULTAR_PEDIDO;
				cliente_rta_CONSULTAR_PEDIDO->parametros=rta_CONSULTAR_PEDIDO;
				enviar_mensaje(cliente_rta_CONSULTAR_PEDIDO,cliente_fd);


		/*
		 * Se le enviará al Módulo Sindicato el mensaje Obtener Pedido especificando un pedido del Restaurante,
		 * con la finalidad de obtener información actualizada del mismo,
		 * como por ejemplo, la cantidad lista.
		 */
		break;
		case 0:
			pthread_exit(NULL);
		case -1:
			pthread_exit(NULL);
		}



}

void* fhilo_plato (t_plato_pcb* v){



}

uint32_t recibir_RTA_GUARDAR_PEDIDO(int socket){

}
uint32_t recibir_RTA_GUARDAR_PLATO(int socket){

}
rta_obtenerPedido* recibir_RTA_OBTENER_PEDIDO (int socket){


}
rta_obtenerReceta* recibir_RTA_OBTENER_RECETA(int socket){


}


void delay (int number_of_seconds){
	int milli_seconds = 1000000* number_of_seconds;
	clock_t start_time = clock();
	while (clock() < start_time + milli_seconds);
}

rta_obtenerRestaurante* metadata_restaurante(int socket){

	int cod_op;
	int id_proceso;
	if(recv(socket, &cod_op, sizeof(int), MSG_WAITALL)==1);
	cod_op=-1;

	recv(socket, &id_proceso, sizeof(uint32_t), MSG_WAITALL);
	int size=0;
	void* stream = recibir_mensaje(socket,&size);

	rta_obtenerRestaurante* restaurante =malloc(sizeof(rta_obtenerRestaurante));

	restaurante = deserializar_mensaje(stream,cod_op);

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
