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

	//id de restaurante
	cfg_id = config_get_int_value(config,"ID");;

}




void iniciar_restaurante(){
	log_info(log_config_ini, "Iniciar restaurante \n");

	int conexion_sindicato;
	void* buffer = NULL;
	//CONEXION SINDICATO

	conexion_sindicato = conectar_con_sindicato();

	log_info(log_config_ini, "ya me conecte \n");
	//GET DE METADATA
	metadata_rest = malloc(sizeof(rta_obtenerRestaurante));

	if(conexion_sindicato != -1){
		//GET DE METADATA A SINDICATO
		log_info(log_config_ini, "entre al if \n");
		//Enviar mje obtener restaurante


		t_mensaje* obt_restaurante = malloc(sizeof(t_mensaje));
		obt_restaurante->tipo_mensaje = OBTENER_RESTAURANTE;
		obt_restaurante->id = cfg_id;


		t_nombre* nombre_restaurante = malloc(sizeof(t_nombre));
		nombre_restaurante->nombre = malloc(strlen(cfg_nombre_restaurante)+1);
		strcpy(nombre_restaurante->nombre, cfg_nombre_restaurante);
		//		nombre_restaurante->largo_nombre=strlen(cfg_nombre_restaurante);

		obt_restaurante->parametros = nombre_restaurante;
		log_info(log_config_ini, "estoy por enviar el mj\n");
		enviar_mensaje(obt_restaurante, conexion_sindicato);
		free_struct_mensaje(nombre_restaurante, OBTENER_RESTAURANTE);
		free(obt_restaurante);
		//Recibir respuesta obtener restaurante

		log_info(log_config_ini, "estoy por recibir el mj\n");

		buffer = recibir_respuesta(conexion_sindicato);
		metadata_rest = buffer;

		liberar_conexion(conexion_sindicato);

		//CREO LAS DISTINTAS COLAS DE READY Y DE ENTRADA SALIDA
		log_info(log_config_ini, "\tIniciar_colas_ready_es \n");
		iniciar_colas_ready_es (metadata_rest);

		//CREAR PROCESO PLANIFICADOR
		pthread_create(&hilo_planificador, NULL,(void*) fhilo_planificador, NULL);

		//CREAR PROCESO SERVIDOR DE CLIENTES
		log_info(log_config_ini, "Iniciar_planificador de platos \n");
		pthread_create(&hilo_servidor_clientes, NULL,(void*) fhilo_servidor_clientes, NULL);

		//ME CONECTO CON APP PARA ENVIAR MI POS Y NOMBRE
		conectarme_con_app();


	}else{


		log_info(log_config_ini, "\tFin proceso, sindicato no disponible \n");
	}

	//INICIO DE LISTAS GLOBALES

	inicio_de_listas_globales();


}


void conectarme_con_app(){

	//iniciar cliente
	socket_app = iniciar_cliente(cfg_ip_app,cfg_puerto_app); //variable global
	log_info(log_config_ini, "Socket con app: %d\n",socket_app);


	if(socket_app !=-1){
		t_mensaje* mensaje_inicial_app = malloc(sizeof(t_mensaje));

		m_restaurante* m_restaurante = malloc(sizeof(m_restaurante));

		m_restaurante->nombre.nombre = string_duplicate(cfg_nombre_restaurante);
		m_restaurante->posicion.x = metadata_rest->posicion.x;
		m_restaurante->posicion.y = metadata_rest->posicion.y;


		//mandar t_mje

		mensaje_inicial_app->id = cfg_id;
		mensaje_inicial_app->tipo_mensaje = POSICION_RESTAUNTE;
		mensaje_inicial_app->parametros = m_restaurante;
		//crear un hilo que reciba mjes de este socket
		pthread_create(&hilo_serve_app, NULL,(void*) fhilo_serve_app, (void*)socket_app);
		log_info(log_config_ini, "Se inicio el socket de escucha con app \n");
		//mando el mj
		enviar_mensaje(mensaje_inicial_app,socket_app);
		free_struct_mensaje(mensaje_inicial_app->parametros, POSICION_RESTAUNTE);
		free(mensaje_inicial_app);
		log_info(log_config_ini, "Se envio el mj a app \n");

	}else{
		log_info(log_config_ini, "App no disponible \n");
	}
}

void* fhilo_serve_app(int socket){
	int rec;
	int cod_op;
	while(1){
		rec = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
		if(rec == -1 || rec == 0 ){
			cod_op = -1;

			pthread_exit(NULL);
		}
		log_info(log_config_ini ,"Recibi un mj de app \n");
		log_info(log_config_ini ,"Se recibio de app cod_op: %d \n",cod_op);

		process_request(cod_op, socket);
	}
}
void inicio_de_listas_globales(){

	//LSITA DE HILOS CLIENTE
	hilos = list_create();

}

void iniciar_colas_ready_es(){

	/*
	 * Creación de colas de planificación
En base a los Hornos y los cocineros con sus afinidades que se obtuvieron, se deberán crear las distintas colas de ready y de entrada salida.
De esta manera, si tengo dos cocineros que tienen como afinidad a Milanesa se deberá crear dos colas de Ready en las cuales todas las milanesas irán a dicha cola y los demás platos irán a otra. Un cocinero con afinidades solo podrá cocinar aquellas recetas a las cuales es afín.
Por otro lado, durante la ejecución de un plato puede darse que se requiera enviarlo al horno, en estos casos existirá una cantidad de Hornos finitos dentro de cada restaurante que funcionarán como entradas salidas visto en la teoría.
	 *
	 */


	log_info(log_config_ini, "Comienza proceso de colas");

	colas_afinidades = list_create();
	t_nombre* afinidad;
	bool _mismo_nombre(t_cola_afinidad* cola1){
		return mismo_nombre(&(cola1->afinidad), afinidad);
	}
	t_cola_afinidad* cola;

	for(int i = 0;	i < metadata_rest->afinidades->elements_count;	i++){
		afinidad = list_get(metadata_rest->afinidades,i);
		pthread_mutex_lock(&cola_afinidades_mtx);
		cola = list_find(colas_afinidades, (void*)_mismo_nombre);
		pthread_mutex_unlock(&cola_afinidades_mtx);
		if(cola == NULL){
			cola = malloc(sizeof(t_cola_afinidad));
			cola->afinidad.nombre = malloc(strlen(afinidad->nombre)+1);
			strcpy(cola->afinidad.nombre, afinidad->nombre);
			cola->cant_cocineros_disp = 0;
			cola->cola = queue_create();
			pthread_mutex_init(&(cola->mutex_cola), NULL);
			sem_init(&(cola->platos_disp), 0 ,0 );
			list_add(colas_afinidades, cola);
		}
		cola->cant_cocineros_disp ++;
	}

	uint32_t cant_cocineros_otros = metadata_rest->cantCocineros - metadata_rest->afinidades->elements_count;
	if(cant_cocineros_otros != 0){
		cola = malloc(sizeof(t_cola_afinidad));
		cola->afinidad.nombre = "Otros";
		cola->cant_cocineros_disp = cant_cocineros_otros;
		cola->cola = queue_create();
		pthread_mutex_init(&(cola->mutex_cola), NULL);
		sem_init(&(cola->platos_disp), 0 ,0 );
		list_add(colas_afinidades, cola);
	}

	for(int i = 0;	i < colas_afinidades->elements_count;	i++){
		cola = list_get(colas_afinidades,i);
		sem_init(&(cola->cocineros_disp), 0, cola->cant_cocineros_disp);
	}

	sem_init(&hornos_disp, 0, metadata_rest->cantHornos);
	sem_init(&platos_a_hornear_sem, 0 , 0);

	pthread_mutex_init(&platos_block_mtx, NULL);
	platos_block = list_create();

	//TODO ver si es necesario

	pthread_mutex_init(&platos_exec_mtx, NULL);
	platos_exec = list_create();

	log_info(log_config_ini, "Fin inicializacion colas de ready \n");
}

bool mismo_nombre(t_nombre* afinidad1, t_nombre* afinidad2){
	return string_equals_ignore_case(afinidad1->nombre,afinidad2->nombre);
}

void* fhilo_planificador(void* v){
	int contador=0;

	while (contador <10){

		log_info(log_config_ini, "\tCONTADOR: %d \n",contador);

		delay(3);
		contador=contador+1;

	}
	return 0;
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

	pthread_create(&hilo,NULL,(void*)serve_client,(void*)cliente);
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
	int id_proceso;
	void* mensaje = NULL;
	recv(cliente_fd, &id_proceso, sizeof(uint32_t), MSG_WAITALL);

	void* buffer = recibir_mensaje(cliente_fd, &size);

	mensaje = deserializar_mensaje(buffer, cod_op);

	if(cod_op != STRC_MENSAJE_VACIO) free(buffer);
	uint32_t confirmacion;

	switch (cod_op) {

	case POSICION_CLIENTE:
		log_info(log_config_ini ,"pos cliente: %d",cod_op);
		enviar_confirmacion(0, cliente_fd, RTA_POSICION_CLIENTE);

		break;

	case CONSULTAR_PLATOS:

		log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PLATOS: ",cod_op);

		t_nombre* nombre_resturante = mensaje;

		//loggear_mensaje_recibido(nombre_resturante, cod_op, log_config_ini);
		log_info(log_config_ini ,"Se esta conusltando por el resto: %s: ",nombre_resturante->nombre);


		//CONECTARME CON SINDICATO y MANDARLE EL MJ, RECIBIR RTA Y RESPONDER AL CLIENTE

		int socket_CONSULTAR_PLATOS = conectar_con_sindicato();

		t_mensaje * cliente_RTA_CONSULTAR_PLATOS = malloc(sizeof(t_mensaje));

		if(socket_CONSULTAR_PLATOS == -1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");
			cliente_RTA_CONSULTAR_PLATOS->id = cfg_id;
			cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje = ERROR;
			enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);
			free(cliente_RTA_CONSULTAR_PLATOS);

			log_info(log_config_ini,"\tSe envio el mj ERROR al cliente \n");

		}else{
			//CARGO EL MJ
			t_mensaje * sindicato_CONSULTAR_PLATOS = malloc(sizeof(t_mensaje));
			sindicato_CONSULTAR_PLATOS->tipo_mensaje = CONSULTAR_PLATOS;
			sindicato_CONSULTAR_PLATOS->id = cfg_id;
			sindicato_CONSULTAR_PLATOS->parametros = nombre_resturante;

			//ENVIO A SINDICATO
			enviar_mensaje(sindicato_CONSULTAR_PLATOS,socket_CONSULTAR_PLATOS);
			free_struct_mensaje(sindicato_CONSULTAR_PLATOS->parametros, CONSULTAR_PLATOS);
			free(sindicato_CONSULTAR_PLATOS);

			//RECIBO RESPUESTA DE SINDICATO
			t_restaurante_y_plato* rta_sindicato_CONSULTAR_PLATOS;
			rta_sindicato_CONSULTAR_PLATOS = recibir_respuesta(socket_CONSULTAR_PLATOS);

			//ENVIAR RESPUESTA DE SINDICATO A CLIENTE

			cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje = RTA_CONSULTAR_PLATOS;
			cliente_RTA_CONSULTAR_PLATOS->parametros = rta_sindicato_CONSULTAR_PLATOS;
			cliente_RTA_CONSULTAR_PLATOS->id = cfg_id;

			enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);
			free_struct_mensaje(cliente_RTA_CONSULTAR_PLATOS->parametros, RTA_CONSULTAR_PLATOS);
			free(cliente_RTA_CONSULTAR_PLATOS);

			log_info(log_config_ini,"\tSe envio el mj al cliente\n");

		}
		liberar_conexion(cliente_fd);

		//free(buffer);
		break;
	case CREAR_PEDIDO:


		log_info(log_config_ini ,"Se recibio el mj CREAR_PEDIDO: ",cod_op);

		//esta es la respuesta al cliente, pero el mj responde el id de pedido
		//uint32_t* ok = malloc(sizeof(uint32_t));
		//*ok = 1; //1 para OK, 0 para FAIL

		//CREAR ID PEDIDO
		pthread_mutex_lock(&mutex_id_pedidos);
		id_pedidos = id_pedidos+1;
		pthread_mutex_unlock(&mutex_id_pedidos);


		uint32_t id_pedido;
		pthread_mutex_lock(&mutex_id_pedidos);
		id_pedido = id_pedidos;
		pthread_mutex_unlock(&mutex_id_pedidos);

		t_mensaje* cliente_rta_CREAR_PEDIDO = malloc(sizeof(t_mensaje));

		//ENVIAR EL ID A SINDICADO PARA GUARDAR PEDIDO

		t_nombre_y_id* mje_GUARDAR_PEDIDO=malloc(sizeof(t_nombre_y_id));

		mje_GUARDAR_PEDIDO->id = id_pedido;
		mje_GUARDAR_PEDIDO->nombre.nombre = cfg_nombre_restaurante;



		t_mensaje* _GUARDAR_PEDIDO = malloc(sizeof(t_mensaje));
		_GUARDAR_PEDIDO->tipo_mensaje = GUARDAR_PEDIDO;
		_GUARDAR_PEDIDO->id = cfg_id;
		_GUARDAR_PEDIDO->parametros = mje_GUARDAR_PEDIDO;


		int socket_GUARDAR_PEDIDO = conectar_con_sindicato();
		if(socket_GUARDAR_PEDIDO==-1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

			//enviar al cliente un codigo de error
			//cliente_rta_CREAR_PEDIDO->parametros=ok_CREAR_PEDIDO;
			cliente_rta_CREAR_PEDIDO->id= cfg_id;
			cliente_rta_CREAR_PEDIDO->tipo_mensaje=ERROR;


			log_info(log_config_ini, "\tEstoy por enviar el mj por sindicato caido \n");
			enviar_mensaje(cliente_rta_CREAR_PEDIDO,cliente_fd);
			log_info(log_config_ini,"\tSe envio el mj ERROR al cliente \n");

		}else{
			enviar_mensaje(_GUARDAR_PEDIDO,socket_GUARDAR_PEDIDO);
			free_struct_mensaje(_GUARDAR_PEDIDO->parametros, GUARDAR_PEDIDO);
			free(_GUARDAR_PEDIDO);

			//RECIBIR RESPUESTA DE SINDICATO
			uint32_t* rta_sindicato_GUARDAR_PEDIDO = recibir_respuesta(socket_GUARDAR_PEDIDO);

			liberar_conexion(socket_GUARDAR_PEDIDO);
			//recibo un ok/fail pero al cliente le envio el id del pedido creado
			//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO
			if((*rta_sindicato_GUARDAR_PEDIDO) == 0){
				uint32_t* ok_CREAR_PEDIDO = malloc(sizeof(uint32_t));
				*ok_CREAR_PEDIDO = id_pedido;
				cliente_rta_CREAR_PEDIDO->tipo_mensaje = RTA_CREAR_PEDIDO;
				cliente_rta_CREAR_PEDIDO->parametros = ok_CREAR_PEDIDO;
				cliente_rta_CREAR_PEDIDO->id = cfg_id;
				enviar_mensaje(cliente_rta_CREAR_PEDIDO, cliente_fd);
				log_info(log_config_ini ,"Se envio: RTA_CREAR_PEDIDO ",cod_op);
				free_struct_mensaje(cliente_rta_CREAR_PEDIDO->parametros, RTA_CREAR_PEDIDO);
				free(cliente_rta_CREAR_PEDIDO);
			}else{
				t_mensaje* msj_error = malloc(sizeof(t_mensaje));
				msj_error->id = cfg_id;
				msj_error->tipo_mensaje = ERROR;
				enviar_mensaje(msj_error, cliente_fd);
				free(msj_error);
			}
		}
		liberar_conexion(cliente_fd);

		break;
	case AGREGAR_PLATO:
		log_info(log_config_ini ,"Se recibio el mj AGREGAR_PLATO: ",cod_op);
		confirmacion = 0;


		//A través del envío del mensaje Guardar Plato al Módulo Sindicato, agrega un plato correspondiente a un pedido específico, que se encontrará relacionado con el
		//Restaurante que envió dicho mensaje. Solo se podrá crear platos sobre pedidos existentes.

		//DESERIALIZO EL MJ

		t_nombre_y_id* mj_agregar_plato = mensaje;
		log_info(log_config_ini ,"Se quiere AGREGAR_PLATO en resto: %s y id: %d",mj_agregar_plato->nombre.nombre,mj_agregar_plato->id);

		//VALIDACION DE ID PEDIDO Y RESTAURANTE
		if(mj_agregar_plato->id <= id_pedidos){
			log_info(log_config_ini, "\tEl pedido corresponde a este restaurante \n");

			//CONEXION CON SINDICATO
			int socket_GUARDAR_PLATO = conectar_con_sindicato();

			if(socket_GUARDAR_PLATO != -1){

				m_guardarPlato* sindicato_GUARDAR_PLATO = malloc(sizeof(m_guardarPlato));
				sindicato_GUARDAR_PLATO->cantidad = 1;

				sindicato_GUARDAR_PLATO->comida.nombre = malloc(strlen(mj_agregar_plato->nombre.nombre)+1);
				strcpy(sindicato_GUARDAR_PLATO->comida.nombre, mj_agregar_plato->nombre.nombre);
				sindicato_GUARDAR_PLATO->idPedido = mj_agregar_plato->id;

				sindicato_GUARDAR_PLATO->restaurante.nombre = malloc(strlen(cfg_nombre_restaurante)+1);
				strcpy(sindicato_GUARDAR_PLATO->restaurante.nombre , cfg_nombre_restaurante);

				t_mensaje* mje_sindicato_GUARDAR_PLATO= malloc(sizeof(t_mensaje));
				mje_sindicato_GUARDAR_PLATO->tipo_mensaje=GUARDAR_PLATO;
				mje_sindicato_GUARDAR_PLATO->parametros=sindicato_GUARDAR_PLATO;
				mje_sindicato_GUARDAR_PLATO->id = cfg_id;

				enviar_mensaje(mje_sindicato_GUARDAR_PLATO,socket_GUARDAR_PLATO);
				free_struct_mensaje(mje_sindicato_GUARDAR_PLATO->parametros, GUARDAR_PLATO);
				free(mje_sindicato_GUARDAR_PLATO);


				//RECIBIR RESPUESTA DE SINDICATO
				uint32_t* rta_sindicato_GUARDAR_PLATO = recibir_respuesta(socket_GUARDAR_PLATO);// TODO

				//recibo un ok/fail y al cliente le mando el mismo ok/fail
				//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO

				confirmacion = *rta_sindicato_GUARDAR_PLATO;
				free_struct_mensaje(rta_sindicato_GUARDAR_PLATO, RTA_GUARDAR_PLATO);

			}


		}else{
			log_info(log_config_ini ,"El id de pedido no corresponde a este restaurante");
		}
		enviar_confirmacion(confirmacion, cliente_fd, RTA_AGREGAR_PLATO);
		liberar_conexion(cliente_fd);
		free_struct_mensaje(mj_agregar_plato, AGREGAR_PLATO);
		break;

	case CONFIRMAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONFIRMAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ

		t_nombre_y_id* id_CONFIRMAR_PEDIDO = mensaje;
		log_info(log_config_ini ,"Se quiere CONFIRMAR_PEDIDO de resto: %s  con id: %d",id_CONFIRMAR_PEDIDO->nombre.nombre,id_CONFIRMAR_PEDIDO->id);


		//1) OBTENER EL PEDIDO DEL MODULO SINDICATO

		//nombre_restaurante_OBTENER_PEDIDO->largo_nombre=strlen(cfg_nombre_restaurante);


		t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
		sindicato_OBTENER_PEDIDO->id=id_CONFIRMAR_PEDIDO->id;
		//sindicato_OBTENER_PEDIDO->nombre.largo_nombre=nombre_restaurante_OBTENER_PEDIDO->largo_nombre;

		sindicato_OBTENER_PEDIDO->nombre.nombre = malloc(strlen(cfg_nombre_restaurante)+1);
		strcpy(sindicato_OBTENER_PEDIDO->nombre.nombre, cfg_nombre_restaurante);



		t_mensaje* mje_sindicato_OBTENER_PEDIDO= malloc(sizeof(t_mensaje));
		mje_sindicato_OBTENER_PEDIDO->tipo_mensaje=OBTENER_PEDIDO;
		mje_sindicato_OBTENER_PEDIDO->parametros=sindicato_OBTENER_PEDIDO;
		mje_sindicato_OBTENER_PEDIDO->id = cfg_id;

		//estructura de respuesta al cliente
		rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO = malloc(sizeof(rta_obtenerPedido));

		//CONEXION CON SINDICATO


		int socket_OBTENER_PEDIDO = conectar_con_sindicato();

		confirmacion = 0;

		if(socket_OBTENER_PEDIDO != -1){

			//ENVIAR OBTENER_PEDIDO A SINDICATO
			enviar_mensaje(mje_sindicato_OBTENER_PEDIDO,socket_OBTENER_PEDIDO);
			free_struct_mensaje(mje_sindicato_OBTENER_PEDIDO, OBTENER_PEDIDO);
			free(mje_sindicato_OBTENER_PEDIDO);
			log_info(log_config_ini ,"Se envio a sindicato el mj OBTENER_PEDIDO: ",cod_op);

			//RECIBIR RESPUESTA DE SINDICATO

			rta_sindicato_RTA_OBTENER_PEDIDO = recibir_respuesta(socket_OBTENER_PEDIDO);// TODO
			log_info(log_config_ini ,"Se recibio rta de sindicato el mj OBTENER_PEDIDO: ",cod_op);
			liberar_conexion(socket_OBTENER_PEDIDO);

			//2) GENERAR EL PCB DE CADA PLATO DED PEDIDO - OBTENER RECETA


			t_elemPedido* plato_n;
			t_nombre* sindicato_nombre_plato_receta;

			t_mensaje* mje_sindicato_OBTENER_RECETA = malloc(sizeof(t_mensaje));
			mje_sindicato_OBTENER_RECETA->tipo_mensaje = OBTENER_RECETA;
			mje_sindicato_OBTENER_RECETA->id = cfg_id;
			rta_obtenerReceta* rta_sindicato_RTA_OBTENER_RECETA = malloc(sizeof(rta_obtenerReceta));
			//t_plato_pcb* plato_pcb = malloc(sizeof(t_plato_pcb));//correccion
			t_plato_pcb* plato_pcb;
			//t_nombre* plato_pcb_nombre_plato = malloc(sizeof(t_nombre));//correccion


			for (int inicio_plato = 0;
					inicio_plato <rta_sindicato_RTA_OBTENER_PEDIDO->cantPedidos;
					inicio_plato++)
			{
				sindicato_nombre_plato_receta = malloc(sizeof(t_nombre));
				plato_pcb = malloc(sizeof(t_plato_pcb));


				plato_n =  list_get(rta_sindicato_RTA_OBTENER_PEDIDO->infoPedidos, inicio_plato);
				//obtengo el nombre del plato
				sindicato_nombre_plato_receta->nombre = malloc(strlen(plato_n->comida.nombre)+1);

				strcpy(sindicato_nombre_plato_receta->nombre, plato_n->comida.nombre);
//				sindicato_nombre_plato_receta->largo_nombre=pedido_n->comida.largo_nombre;

				//cargo el mj a enviar a sindicato para pedir receta
				mje_sindicato_OBTENER_RECETA->parametros = sindicato_nombre_plato_receta;

				//conexion sindicato - TODO NO HAGO VALIDACION DE SINDICATO DISPONIBLE
				int socket_OBTENER_RECETA = conectar_con_sindicato();

				enviar_mensaje(mje_sindicato_OBTENER_RECETA,socket_OBTENER_RECETA);
				free_struct_mensaje(mje_sindicato_OBTENER_RECETA, OBTENER_RECETA);
				free(mje_sindicato_OBTENER_RECETA);

				//RECIBIR RESPUESTA DE SINDICATO
				rta_sindicato_RTA_OBTENER_RECETA = recibir_respuesta(socket_OBTENER_RECETA);// TODO

				liberar_conexion(socket_OBTENER_RECETA);


				//cargo el pcb
				plato_pcb->id_pedido = id_CONFIRMAR_PEDIDO->id;
				plato_pcb->comida.nombre = malloc(strlen(plato_n->comida.nombre)+1);
				strcpy(plato_pcb->comida.nombre,plato_n->comida.nombre);

				plato_pcb->cantTotal = plato_n->cantTotal;
				plato_pcb->cantHecha = plato_n->cantHecha;
				plato_pcb->cantPasos = rta_sindicato_RTA_OBTENER_RECETA->cantPasos;
				plato_pcb->pasos = rta_sindicato_RTA_OBTENER_RECETA->pasos;

				free_struct_mensaje(rta_sindicato_RTA_OBTENER_RECETA, RTA_OBTENER_RECETA);

				//cargo lista de pcb
				log_info(log_config_ini,"\tPCB id: %d \n",plato_pcb->id_pedido);
				log_info(log_config_ini,"\tPCB nombe plato: %s \n",plato_pcb->comida.nombre);
				log_info(log_config_ini,"\tPCB cant de platos: %d \n",plato_pcb->cantTotal);
				log_info(log_config_ini,"\tPCB cant hecha: %d \n",plato_pcb->cantHecha);
				log_info(log_config_ini,"\tPCB cant pasos: %d \n",plato_pcb->cantPasos);
				//log_info(log_config_ini,"\tPCB cant pasos: %d \n",plato_pcb->cantPasos); PASOS list get

//				pthread_mutex_lock(&mutex_pcb);
//				list_add(pcb_platos, plato_pcb);
//				pthread_mutex_unlock(&mutex_pcb);
				agregar_cola_ready(plato_pcb);

			}

			free_struct_mensaje(id_CONFIRMAR_PEDIDO, CONFIRMAR_PEDIDO);
			free_struct_mensaje(rta_sindicato_RTA_OBTENER_PEDIDO, RTA_OBTENER_PEDIDO);

			//respondo al cliente -- consultar en que casos se debe mandar fail
			//esta es la respuesta al cliente,
			confirmacion = 1;

		}
		//RESPONDO AL CLIENTE
		//OK o FAIL
		enviar_confirmacion(confirmacion,cliente_fd, RTA_CONFIRMAR_PEDIDO);

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

		liberar_conexion(cliente_fd);
		break;
	case CONSULTAR_PEDIDO:
		log_info(log_config_ini ,"Se recibio el mj CONSULTAR_PEDIDO: ",cod_op);

		//DESERIALIZO EL MJ
		uint32_t* id_CONSULTAR_PEDIDO = mensaje;
		log_info(log_config_ini ,"CONSULTAR_PEDIDO con id: %d",*(id_CONSULTAR_PEDIDO));
		//respuesta cliente

		t_mensaje* cliente_rta_CONSULTAR_PEDIDO = malloc(sizeof(t_mensaje));

		//CONEXION CON SINDICATO
		int socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO = conectar_con_sindicato();

		if(socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO == -1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

			//enviar al cliente un codigo de error
			cliente_rta_CONSULTAR_PEDIDO->id=cfg_id;
			cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje = ERROR;
			enviar_mensaje(cliente_rta_CONSULTAR_PEDIDO,cliente_fd);
			log_info(log_config_ini,"\tSe envio el mj ERROR al cliente \n");


		}else{

			//ENVIAR MJ A SINDICATO - OBTENER PEDIDO
			//1) OBTENER EL PEDIDO DEL MODULO SINDICATO
			t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
			sindicato_OBTENER_PEDIDO->id = *id_CONSULTAR_PEDIDO;

			sindicato_OBTENER_PEDIDO->nombre.nombre = malloc(strlen(cfg_nombre_restaurante));
			strcpy(sindicato_OBTENER_PEDIDO->nombre.nombre, cfg_nombre_restaurante);


			t_mensaje* mje_sindicato_OBTENER_PEDIDO = malloc(sizeof(t_mensaje));
			mje_sindicato_OBTENER_PEDIDO->tipo_mensaje = OBTENER_PEDIDO;
			mje_sindicato_OBTENER_PEDIDO->parametros = sindicato_OBTENER_PEDIDO;
			enviar_mensaje(mje_sindicato_OBTENER_PEDIDO,socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);
			free_struct_mensaje(mje_sindicato_OBTENER_PEDIDO->parametros, OBTENER_PEDIDO);
			free(mje_sindicato_OBTENER_PEDIDO);


			//RECIBIR RESPUESTA DE SINDICATO
			rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO = recibir_respuesta(socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);// TODO

			liberar_conexion( socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);

			rta_consultarPedido* rta_CONSULTAR_PEDIDO = malloc(sizeof(rta_consultarPedido));

			rta_CONSULTAR_PEDIDO->restaurante.nombre = malloc(strlen(cfg_nombre_restaurante));
			strcpy(rta_CONSULTAR_PEDIDO->restaurante.nombre, cfg_nombre_restaurante);

			rta_CONSULTAR_PEDIDO->cantPlatos = rta_sindicato_RTA_OBTENER_PEDIDO->cantPedidos;
			rta_CONSULTAR_PEDIDO->platos = list_duplicate(rta_sindicato_RTA_OBTENER_PEDIDO->infoPedidos);

			cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje = RTA_CONSULTAR_PEDIDO;
			cliente_rta_CONSULTAR_PEDIDO->parametros = rta_CONSULTAR_PEDIDO;
			cliente_rta_CONSULTAR_PEDIDO->id = cfg_id;
			enviar_mensaje(cliente_rta_CONSULTAR_PEDIDO,cliente_fd);
			free_struct_mensaje(cliente_rta_CONSULTAR_PEDIDO->parametros, RTA_CONSULTAR_PEDIDO);
			free(cliente_rta_CONSULTAR_PEDIDO);
		}

		liberar_conexion(cliente_fd);
		break;
	case 0:
		pthread_exit(NULL);
	case -1:
		pthread_exit(NULL);
	}

}



void agregar_cola_ready(t_plato_pcb* plato){
	t_cola_afinidad* cola_afinidad;
	t_nombre* afinidad = &(plato->comida);
	bool _mismo_nombre(t_cola_afinidad* cola){
		return mismo_nombre(afinidad, &(cola->afinidad));
	}
	cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);
	if(cola_afinidad == NULL){
		afinidad = malloc(sizeof(t_nombre));
		afinidad->nombre = "Otros";
		cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);
		free(afinidad);
	}
	pthread_mutex_lock(&(cola_afinidad->mutex_cola));
	queue_push(cola_afinidad->cola, plato);
	//LE AVISO AL HILO PLANIFICADOR QUE TIENE UN PEDIDO CON PLATOS A PLANIFICAR
	sem_post(&(cola_afinidad->platos_disp));
	pthread_mutex_unlock(&(cola_afinidad->mutex_cola));
}

void enviar_confirmacion(uint32_t _confirmacion, int cliente, op_code cod_op){
	t_mensaje* mensaje_a_enviar = malloc(sizeof(t_mensaje));
	mensaje_a_enviar->tipo_mensaje = cod_op;
	mensaje_a_enviar->id = cfg_id;
	uint32_t* confirmacion = malloc(sizeof(uint32_t));
	*confirmacion = _confirmacion;
	mensaje_a_enviar->parametros = confirmacion;
	enviar_mensaje(mensaje_a_enviar, cliente);
	loggear_mensaje_enviado(confirmacion, cod_op, log_config_ini);
	free_struct_mensaje(confirmacion,cod_op);
	free(mensaje_a_enviar);
}




void delay (int number_of_seconds){
	int milli_seconds = 1000000* number_of_seconds;
	clock_t start_time = clock();
	while (clock() < start_time + milli_seconds);
}

void* recibir_respuesta(int socket){
	op_code cod_op = 999;
	uint32_t id_proceso;
	void* mensaje=NULL;
	int error = 0;
	int _recv = recv(socket, &cod_op, sizeof(op_code), MSG_WAITALL);
	log_info(log_config_ini, "cod op: %d\n",cod_op);

	if(_recv == 0 || _recv == -1){
		//error
	}

	_recv = recv(socket, &id_proceso, sizeof(uint32_t), MSG_WAITALL);

	if(_recv == 0 || _recv == -1){
		//error
	}

	void* buffer = recibir_mensaje(socket, &error);

	if(error == 1){
		//error
	}

	mensaje = deserializar_mensaje(buffer, cod_op);
	loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);
	free(buffer);
	return mensaje;
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

