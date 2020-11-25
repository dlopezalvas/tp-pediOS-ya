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

	cfg_retardo_ciclo_CPU = config_get_int_value(config,"RETARDO_CPU");
	log_info(log_config_ini, "\t\tcfg_retardo_cpu: %d \n",cfg_quantum);

	//id de restaurante
	cfg_id = config_get_int_value(config,"ID");

}




void iniciar_restaurante(){
	log_info(log_config_ini, "Iniciar restaurante \n");

	int conexion_sindicato;

	//CONEXION SINDICATO

	conexion_sindicato = conectar_con_sindicato();

	log_info(log_config_ini, "ya me conecte \n");
	//GET DE METADATA


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
		loggear_mensaje_enviado(obt_restaurante->parametros, obt_restaurante->tipo_mensaje, log_config_ini);

		free_struct_mensaje(nombre_restaurante, OBTENER_RESTAURANTE);
		free(obt_restaurante);
		//Recibir respuesta obtener restaurante

		log_info(log_config_ini, "estoy por recibir el mj\n");
		uint32_t cod_op;
		metadata_rest = recibir_respuesta(conexion_sindicato, &cod_op);
		liberar_conexion(conexion_sindicato);

		if(cod_op!= ERROR){
			//CREO LAS DISTINTAS COLAS DE READY Y DE ENTRADA SALIDA
			log_info(log_config_ini, "\tIniciar_colas_ready_es \n");
			inicio_de_listas_globales();
			iniciar_colas_ready_es (metadata_rest);

			//CREAR PROCESO PLANIFICADOR
			pthread_t hiloClock;
			pthread_create(&hiloClock, NULL,(void*) fhilo_clock, NULL);



			//CREAR PROCESO SERVIDOR DE CLIENTES
			log_info(log_config_ini, "Iniciar_planificador de platos \n");
			pthread_create(&hilo_servidor_clientes, NULL,(void*) fhilo_servidor_clientes, NULL);

			//ME CONECTO CON APP PARA ENVIAR MI POS Y NOMBRE
			conectarme_con_app();
		}else{
			log_info(log_config_ini, "\tFin proceso, sindicato respondio error obtener restaurante \n");

		}



	}else{


		log_info(log_config_ini, "\tFin proceso, sindicato no disponible \n");
	}



}


void conectarme_con_app(){

	//iniciar cliente
	socket_app = iniciar_cliente(cfg_ip_app,cfg_puerto_app); //variable global
	log_info(log_config_ini, "Socket con app: %d\n",socket_app);


	if(socket_app !=-1){
		t_mensaje* mensaje_inicial_app = malloc(sizeof(t_mensaje));

		m_restaurante* mje_restaurante = malloc(sizeof(m_restaurante));

		mje_restaurante->nombre.nombre = string_duplicate(cfg_nombre_restaurante);
		mje_restaurante->posicion.x = metadata_rest->posicion.x;
		mje_restaurante->posicion.y = metadata_rest->posicion.y;


		//mandar t_mje

		mensaje_inicial_app->id = cfg_id;
		mensaje_inicial_app->tipo_mensaje = POSICION_RESTAUNTE;
		mensaje_inicial_app->parametros = mje_restaurante;
		//crear un hilo que reciba mjes de este socket
		pthread_create(&hilo_serve_app, NULL,(void*) fhilo_serve_app, (void*)socket_app);
		log_info(log_config_ini, "Se inicio el socket de escucha con app \n");
		//mando el mj
		enviar_mensaje(mensaje_inicial_app,socket_app);
		free_struct_mensaje(mensaje_inicial_app->parametros, POSICION_RESTAUNTE);
		free(mensaje_inicial_app);
		log_info(log_config_ini, "Se envio el handshake a app \n");

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
	//INICIO DE LISTAS GLOBALES

	pthread_mutex_init(&id_plato_global_mtx, NULL);
	id_plato_global = 0;

	//LSITA DE HILOS CLIENTE
	hilos = list_create();
	hilos_reposo = list_create();


	hornos_disp = queue_create();
	list_pedidos_confirm= list_create();
	pthread_mutex_init(&list_pedidos_confirm_mtx, NULL);

	platos_EXEC = list_create();
	platos_REPOSANDO = list_create();
	platos_HORNEANDO = list_create();

	pthread_mutex_init(&mutex_EXEC, NULL);
	pthread_mutex_init(&mutex_HORNEANDO, NULL);
	pthread_mutex_init(&mutex_REPOSANDO, NULL);
	pthread_mutex_init(&hornos_disp_mtx, NULL);



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
			cola->ready = queue_create();
			cola->cola_cocineros_disp = queue_create();
			cola->cola_cocineros_exec = queue_create();
			pthread_mutex_init(&(cola->cola_ready_mtx), NULL);
			pthread_mutex_init(&(cola->cola_cocineros_disp_mtx), NULL);
			pthread_mutex_init(&(cola->cola_cocineros_exec_mtx), NULL);
			sem_init(&(cola->platos_disp), 0 ,0 );
			sem_init(&(cola->sem_exec), 0, 0);
			list_add(colas_afinidades, cola);
		}
		cola->cant_cocineros_disp ++;
	}

	pthread_t hilo_horno;
	t_horno* horno;
	pthread_mutex_init(&ready_hornos_mtx, NULL);
	pthread_mutex_init(&hornos_disp_mtx, NULL);
	ready_hornos = queue_create();
	for(int i = 0; i < metadata_rest->cantHornos; i ++){
		horno = malloc(sizeof(t_horno));
		pthread_mutex_init(&(horno->mtx_IO), NULL);
		pthread_mutex_lock(&(horno->mtx_IO));
		horno->plato_a_cocinar = NULL;
		pthread_create(&hilo_horno, NULL,(void*) hornear, (void*)horno);
		horno->hilo = hilo_horno;
		queue_push(hornos_disp, horno);
	}
	sem_init(&hornos_disp_sem, 0, metadata_rest->cantHornos);
	sem_init(&sem_ready_hornos, 0, 0);

	pthread_create(&hilo_horno, NULL,(void*) planificar_hornos, NULL);


	uint32_t cant_cocineros_otros = metadata_rest->cantCocineros - metadata_rest->afinidades->elements_count;
	if(cant_cocineros_otros != 0){
		cola = malloc(sizeof(t_cola_afinidad));
		cola->afinidad.nombre = "Otros";
		cola->cant_cocineros_disp = cant_cocineros_otros;
		cola->ready = queue_create();
		cola->cola_cocineros_disp = queue_create();
		cola->cola_cocineros_exec = queue_create();
		pthread_mutex_init(&(cola->cola_ready_mtx), NULL);
		pthread_mutex_init(&(cola->cola_cocineros_disp_mtx), NULL);
		pthread_mutex_init(&(cola->cola_cocineros_exec_mtx), NULL);
		sem_init(&(cola->platos_disp), 0 ,0 );
		sem_init(&(cola->sem_exec), 0, 0);
		list_add(colas_afinidades, cola);
	}
	pthread_t hilo_cocinero;
	pthread_t hilo_planificador2;
	t_cocinero* cocinero;
	for(int i = 0;	i < colas_afinidades->elements_count;	i++){
		cola = list_get(colas_afinidades,i);
		sem_init(&(cola->cocineros_disp_sem), 0, cola->cant_cocineros_disp);
		for(int j = 0; j < cola->cant_cocineros_disp; j++){
			cocinero = malloc(sizeof(t_cocinero));
			pthread_mutex_init(&(cocinero->mtx_exec), NULL);
			pthread_mutex_lock(&(cocinero->mtx_exec));
			cocinero->plato_a_cocinar = NULL;
			cocinero->ciclos_ejecutando = 0;
			pthread_create(&hilo_cocinero, NULL,(void*) cocinar, (void*)cocinero);
			cocinero->hilo = hilo_cocinero;
			queue_push(cola->cola_cocineros_disp, cocinero);
		}
		pthread_create(&hilo_planificador, NULL,(void*) planificador_ready_a_exec, (void*)cola);
		pthread_create(&hilo_planificador2, NULL,(void*) planificador_exec, (void*)cola);
	}




	//	pthread_mutex_init(&platos_block_mtx, NULL);
	//	platos_block = list_create();

	//TODO poner semaforo para que termine de inicialziar antes de planificar

	log_info(log_config_ini, "Fin inicializacion colas de ready \n");
}

bool mismo_nombre(t_nombre* afinidad1, t_nombre* afinidad2){
	return string_equals_ignore_case(afinidad1->nombre,afinidad2->nombre);
}

bool mismo_id(uint32_t id1, uint32_t id2){
	return id1 == id2;
}

//void* fhilo_planificador(void* v){
//	int contador=0;
//
//	while (contador <10){
//
//		log_info(log_config_ini, "\tCONTADOR: %d \n",contador);
//
//		delay(3);
//		contador=contador+1;
//
//	}
//	return 0;
//}



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

	uint32_t confirmado;
	bool esta_confirmado;

	bool _mismo_id(uint32_t _id){
		return mismo_id(_id, confirmado);
	}


	recv(cliente_fd, &id_proceso, sizeof(uint32_t), MSG_WAITALL);

	void* buffer = recibir_mensaje(cliente_fd, &size);

	mensaje = deserializar_mensaje(buffer, cod_op);


	if(cod_op != STRC_MENSAJE_VACIO) free(buffer);
	t_confirmacion confirmacion;


	switch (cod_op) {

	case POSICION_CLIENTE:
		log_info(log_config_ini ,"pos cliente: %d",cod_op);
		enviar_confirmacion(FAIL, cliente_fd, RTA_POSICION_CLIENTE);

		break;

	case CONSULTAR_PLATOS:

		loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);

		//t_nombre* nombre_resturante = mensaje;
		t_nombre* nombre_resturante=malloc(sizeof(t_nombre));
		nombre_resturante->nombre= string_duplicate(cfg_nombre_restaurante);

		log_info(log_config_ini ,"Se esta conusltando por el resto: %s: ",nombre_resturante->nombre);


		//CONECTARME CON SINDICATO y MANDARLE EL MJ, RECIBIR RTA Y RESPONDER AL CLIENTE

		int socket_CONSULTAR_PLATOS = conectar_con_sindicato();

		t_mensaje * cliente_RTA_CONSULTAR_PLATOS = malloc(sizeof(t_mensaje));

		if(socket_CONSULTAR_PLATOS == -1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");
			cliente_RTA_CONSULTAR_PLATOS->id = cfg_id;
			cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje = ERROR;
			enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);

			loggear_mensaje_enviado(cliente_RTA_CONSULTAR_PLATOS->parametros, cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje, log_config_ini);

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

			loggear_mensaje_enviado(sindicato_CONSULTAR_PLATOS->parametros, sindicato_CONSULTAR_PLATOS->tipo_mensaje, log_config_ini);
			//free_struct_mensaje(sindicato_CONSULTAR_PLATOS->parametros, CONSULTAR_PLATOS);
			free(sindicato_CONSULTAR_PLATOS);


			//RECIBO RESPUESTA DE SINDICATO
			t_restaurante_y_plato* rta_sindicato_CONSULTAR_PLATOS;
			uint32_t error_cod_op;
			rta_sindicato_CONSULTAR_PLATOS = recibir_respuesta(socket_CONSULTAR_PLATOS, &error_cod_op);

			if(error_cod_op!=ERROR){
				//ENVIAR RESPUESTA DE SINDICATO A CLIENTE

				cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje = RTA_CONSULTAR_PLATOS;
				cliente_RTA_CONSULTAR_PLATOS->parametros = rta_sindicato_CONSULTAR_PLATOS;
				cliente_RTA_CONSULTAR_PLATOS->id = cfg_id;

				enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);

				loggear_mensaje_enviado(cliente_RTA_CONSULTAR_PLATOS->parametros, cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje, log_config_ini);

				//free_struct_mensaje(cliente_RTA_CONSULTAR_PLATOS->parametros, RTA_CONSULTAR_PLATOS);
				free(cliente_RTA_CONSULTAR_PLATOS);
				free_struct_mensaje(rta_sindicato_CONSULTAR_PLATOS,RTA_CONSULTAR_PLATOS);

				log_info(log_config_ini,"\tSe envio el mj al cliente\n");

			}else{

				log_info(log_config_ini, "\tsindicato respondio error a consultar platos\n");
				cliente_RTA_CONSULTAR_PLATOS->id = cfg_id;
				cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje = ERROR;
				enviar_mensaje(cliente_RTA_CONSULTAR_PLATOS,cliente_fd);

				loggear_mensaje_enviado(cliente_RTA_CONSULTAR_PLATOS->parametros, cliente_RTA_CONSULTAR_PLATOS->tipo_mensaje, log_config_ini);

				free(cliente_RTA_CONSULTAR_PLATOS);

				log_info(log_config_ini,"\tSe envio el mj ERROR al cliente \n");
			}



		}
		free(nombre_resturante->nombre);
		free(nombre_resturante);
		free(mensaje);

		//free(buffer);
		break;
	case CREAR_PEDIDO:


		loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);

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



		//ENVIAR EL ID A SINDICADO PARA GUARDAR PEDIDO

		t_nombre_y_id* mje_GUARDAR_PEDIDO=malloc(sizeof(t_nombre_y_id));

		mje_GUARDAR_PEDIDO->id = id_pedido;
		mje_GUARDAR_PEDIDO->nombre.nombre = string_duplicate(cfg_nombre_restaurante);



		t_mensaje* _GUARDAR_PEDIDO = malloc(sizeof(t_mensaje));
		_GUARDAR_PEDIDO->tipo_mensaje = GUARDAR_PEDIDO;
		_GUARDAR_PEDIDO->id = cfg_id;
		_GUARDAR_PEDIDO->parametros = mje_GUARDAR_PEDIDO;


		t_mensaje* cliente_rta_CREAR_PEDIDO = malloc(sizeof(t_mensaje));
		int socket_GUARDAR_PEDIDO = conectar_con_sindicato();
		if(socket_GUARDAR_PEDIDO==-1){

			log_info(log_config_ini, "\tNo se pudo conectar con sindicato, se envia respuesta default \n");

			//enviar al cliente un codigo de error
			//cliente_rta_CREAR_PEDIDO->parametros=ok_CREAR_PEDIDO;
			cliente_rta_CREAR_PEDIDO->id= cfg_id;
			cliente_rta_CREAR_PEDIDO->tipo_mensaje=ERROR;


			log_info(log_config_ini, "\tEstoy por enviar el mj por sindicato caido \n");
			enviar_mensaje(cliente_rta_CREAR_PEDIDO,cliente_fd);

			loggear_mensaje_enviado(cliente_rta_CREAR_PEDIDO->parametros, cliente_rta_CREAR_PEDIDO->tipo_mensaje, log_config_ini);
			free(cliente_rta_CREAR_PEDIDO);

		}else{
			enviar_mensaje(_GUARDAR_PEDIDO,socket_GUARDAR_PEDIDO);

			loggear_mensaje_enviado(_GUARDAR_PEDIDO->parametros, _GUARDAR_PEDIDO->tipo_mensaje, log_config_ini);

			free_struct_mensaje(_GUARDAR_PEDIDO->parametros, GUARDAR_PEDIDO);
			free(_GUARDAR_PEDIDO);

			//RECIBIR RESPUESTA DE SINDICATO
			uint32_t error_cod_op;
			uint32_t* rta_sindicato_GUARDAR_PEDIDO = recibir_respuesta(socket_GUARDAR_PEDIDO, &error_cod_op);

			liberar_conexion(socket_GUARDAR_PEDIDO);
			//recibo un ok/fail pero al cliente le envio el id del pedido creado
			//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO
			if((*rta_sindicato_GUARDAR_PEDIDO) == OK){
				uint32_t* ok_CREAR_PEDIDO = malloc(sizeof(uint32_t));
				*ok_CREAR_PEDIDO = id_pedido;
				cliente_rta_CREAR_PEDIDO->tipo_mensaje = RTA_CREAR_PEDIDO;
				cliente_rta_CREAR_PEDIDO->parametros = ok_CREAR_PEDIDO;
				cliente_rta_CREAR_PEDIDO->id = cfg_id;
				enviar_mensaje(cliente_rta_CREAR_PEDIDO, cliente_fd);
				loggear_mensaje_enviado(cliente_rta_CREAR_PEDIDO->parametros, cliente_rta_CREAR_PEDIDO->tipo_mensaje, log_config_ini);
				free_struct_mensaje(cliente_rta_CREAR_PEDIDO->parametros,RTA_CREAR_PEDIDO);
			}else{
				t_mensaje* msj_error = malloc(sizeof(t_mensaje));
				msj_error->id = cfg_id;
				msj_error->tipo_mensaje = ERROR;
				enviar_mensaje(msj_error, cliente_fd);
				loggear_mensaje_enviado(msj_error->parametros, msj_error->tipo_mensaje, log_config_ini);

				free(msj_error);

			}
			free_struct_mensaje(rta_sindicato_GUARDAR_PEDIDO,RTA_GUARDAR_PEDIDO);

			free(cliente_rta_CREAR_PEDIDO);
		}

		free(mensaje);

		break;
	case AGREGAR_PLATO:
		loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);
		confirmacion = FAIL;

		//A través del envío del mensaje Guardar Plato al Módulo Sindicato, agrega un plato correspondiente a un pedido específico, que se encontrará relacionado con el
		//Restaurante que envió dicho mensaje. Solo se podrá crear platos sobre pedidos existentes.

		//DESERIALIZO EL MJ

		t_nombre_y_id* mj_agregar_plato = mensaje;
		log_info(log_config_ini ,"Se quiere AGREGAR_PLATO en resto: %s y id: %d",mj_agregar_plato->nombre.nombre,mj_agregar_plato->id);

		confirmado = mj_agregar_plato->id;

		pthread_mutex_lock(&list_pedidos_confirm_mtx);
		esta_confirmado = list_any_satisfy(list_pedidos_confirm, (void*) _mismo_id);
		pthread_mutex_unlock(&list_pedidos_confirm_mtx);

		//VALIDACION DE ID PEDIDO Y RESTAURANTE
		if((mj_agregar_plato->id <= id_pedidos)&& !esta_confirmado){
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
				loggear_mensaje_enviado(mje_sindicato_GUARDAR_PLATO->parametros, mje_sindicato_GUARDAR_PLATO->tipo_mensaje, log_config_ini);

				free_struct_mensaje(mje_sindicato_GUARDAR_PLATO->parametros, GUARDAR_PLATO);
				free(mje_sindicato_GUARDAR_PLATO);

				uint32_t error_cod_op;
				//RECIBIR RESPUESTA DE SINDICATO
				uint32_t* rta_sindicato_GUARDAR_PLATO = recibir_respuesta(socket_GUARDAR_PLATO, &error_cod_op);// TODO

				//recibo un ok/fail y al cliente le mando el mismo ok/fail
				//ENVIAR RESTA AL CLIENTE CON EL ID DEL PEDIDO CREADO

				confirmacion = *rta_sindicato_GUARDAR_PLATO;
				free_struct_mensaje(rta_sindicato_GUARDAR_PLATO, RTA_GUARDAR_PLATO);

			}


		}else{
			log_info(log_config_ini ,"El id de pedido no corresponde a este restaurante o el pedido ya fue confirmado \n");

		}
		enviar_confirmacion(confirmacion, cliente_fd, RTA_AGREGAR_PLATO);
		// liberar_conexion(cliente_fd);
		free(mj_agregar_plato->nombre.nombre);

		free(mensaje);
		break;

	case CONFIRMAR_PEDIDO:

		//DESERIALIZO EL MJ

		loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);
		t_nombre_y_id* id_CONFIRMAR_PEDIDO = mensaje;
		log_info(log_config_ini ,"Se quiere CONFIRMAR_PEDIDO de resto: %s  con id: %d",id_CONFIRMAR_PEDIDO->nombre.nombre,id_CONFIRMAR_PEDIDO->id);


		//1) OBTENER EL PEDIDO DEL MODULO SINDICATO


		//estructura de respuesta al cliente
		rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO;



		//CONEXION CON SINDICATO
		confirmacion = FAIL;
		//VALIDAR QUE NO SE CONFIRME UN PEDIDO YA CONFIRMADO

		confirmado = id_CONFIRMAR_PEDIDO->id;


		pthread_mutex_lock(&list_pedidos_confirm_mtx);
		esta_confirmado = list_any_satisfy(list_pedidos_confirm, (void*) _mismo_id);
		pthread_mutex_unlock(&list_pedidos_confirm_mtx);

		if((id_CONFIRMAR_PEDIDO->id <= id_pedidos) && !esta_confirmado){

			int socket_OBTENER_PEDIDO = conectar_con_sindicato();



			if(socket_OBTENER_PEDIDO != -1){
				t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
				sindicato_OBTENER_PEDIDO->id=id_CONFIRMAR_PEDIDO->id;
				//sindicato_OBTENER_PEDIDO->nombre.largo_nombre=nombre_restaurante_OBTENER_PEDIDO->largo_nombre;

				sindicato_OBTENER_PEDIDO->nombre.nombre = malloc(strlen(cfg_nombre_restaurante)+1);
				strcpy(sindicato_OBTENER_PEDIDO->nombre.nombre, cfg_nombre_restaurante);

				t_mensaje* mje_sindicato_OBTENER_PEDIDO= malloc(sizeof(t_mensaje));
				mje_sindicato_OBTENER_PEDIDO->tipo_mensaje=OBTENER_PEDIDO;
				mje_sindicato_OBTENER_PEDIDO->parametros=sindicato_OBTENER_PEDIDO;
				mje_sindicato_OBTENER_PEDIDO->id = cfg_id;



				//ENVIAR OBTENER_PEDIDO A SINDICATO
				enviar_mensaje(mje_sindicato_OBTENER_PEDIDO,socket_OBTENER_PEDIDO);
				loggear_mensaje_enviado(mje_sindicato_OBTENER_PEDIDO->parametros, mje_sindicato_OBTENER_PEDIDO->tipo_mensaje, log_config_ini);

				free_struct_mensaje(mje_sindicato_OBTENER_PEDIDO->parametros, OBTENER_PEDIDO);
				free(mje_sindicato_OBTENER_PEDIDO);

				//RECIBIR RESPUESTA DE SINDICATO
				uint32_t error_cod_op;
				rta_sindicato_RTA_OBTENER_PEDIDO = recibir_respuesta(socket_OBTENER_PEDIDO, &error_cod_op);// TODO
				liberar_conexion(socket_OBTENER_PEDIDO);

				//VALIDAR QUE EL SINDICATO NO DEVUELVA ERROR
				if(error_cod_op!=ERROR){
					//2) GENERAR EL PCB DE CADA PLATO DED PEDIDO - OBTENER RECETA


					t_elemPedido* plato_n;
					t_nombre* sindicato_nombre_plato_receta;

					t_mensaje* mje_sindicato_OBTENER_RECETA;
					rta_obtenerReceta* rta_sindicato_RTA_OBTENER_RECETA = malloc(sizeof(rta_obtenerReceta));
					//t_plato_pcb* plato_pcb = malloc(sizeof(t_plato_pcb));//correccion
					t_plato_pcb* plato_pcb;
					//t_nombre* plato_pcb_nombre_plato = malloc(sizeof(t_nombre));//correccion
					pthread_mutex_t clock_plato;

					for (int inicio_plato = 0;
							inicio_plato <rta_sindicato_RTA_OBTENER_PEDIDO->cantPedidos;
							inicio_plato++)
					{
						sindicato_nombre_plato_receta = malloc(sizeof(t_nombre));
						plato_pcb = malloc(sizeof(t_plato_pcb));
						mje_sindicato_OBTENER_RECETA = malloc(sizeof(t_mensaje));
						mje_sindicato_OBTENER_RECETA->tipo_mensaje = OBTENER_RECETA;
						mje_sindicato_OBTENER_RECETA->id = cfg_id;


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
						loggear_mensaje_enviado(mje_sindicato_OBTENER_RECETA->parametros, mje_sindicato_OBTENER_RECETA->tipo_mensaje, log_config_ini);

						free_struct_mensaje(mje_sindicato_OBTENER_RECETA->parametros, OBTENER_RECETA);
						free(mje_sindicato_OBTENER_RECETA);

						//RECIBIR RESPUESTA DE SINDICATO
						rta_sindicato_RTA_OBTENER_RECETA = recibir_respuesta(socket_OBTENER_RECETA, &error_cod_op);// TODO

						liberar_conexion(socket_OBTENER_RECETA);

						for (int inicio_plato_cant = 0;
								inicio_plato_cant <plato_n->cantTotal-plato_n->cantHecha;
								inicio_plato_cant++)
						{
							//cargo el pcb
							plato_pcb->id_pedido = id_CONFIRMAR_PEDIDO->id;
							plato_pcb->comida.nombre = malloc(strlen(plato_n->comida.nombre)+1);
							strcpy(plato_pcb->comida.nombre,plato_n->comida.nombre);

							plato_pcb->cantTotal = plato_n->cantTotal;
							plato_pcb->cantHecha = plato_n->cantHecha;
							plato_pcb->cantPasos = rta_sindicato_RTA_OBTENER_RECETA->cantPasos;
							plato_pcb->pasos = list_duplicate(rta_sindicato_RTA_OBTENER_RECETA->pasos);
							plato_pcb->id_plato = id_plato_global;
							plato_pcb->estado = NEW;

							pthread_mutex_init(&clock_plato, NULL);
							plato_pcb->mutex_clock = clock_plato;

							pthread_mutex_lock(&id_plato_global_mtx);
							id_plato_global++;
							pthread_mutex_unlock(&id_plato_global_mtx);

							//					free_struct_mensaje(rta_sindicato_RTA_OBTENER_RECETA, RTA_OBTENER_RECETA); TODO list duplicate

							//cargo lista de pcb
							log_debug(log_oficial, "[CREAR_PLATO]: Se creo el PCB del plato %s con ID: %d ", plato_pcb->comida.nombre, plato_pcb->id_plato);

							log_info(log_config_ini,"\tPCB id: %d \n",plato_pcb->id_pedido);
							log_info(log_config_ini,"\tPCB nombe plato: %s \n",plato_pcb->comida.nombre);
							log_info(log_config_ini,"\tPCB cant de platos: %d \n",plato_pcb->cantTotal);
							log_info(log_config_ini,"\tPCB cant hecha: %d \n",plato_pcb->cantHecha);
							log_info(log_config_ini,"\tPCB cant pasos: %d \n",plato_pcb->cantPasos);
							//log_info(log_config_ini,"\tPCB cant pasos: %d \n",plato_pcb->cantPasos); PASOS list get

							pthread_mutex_lock(&list_pedidos_confirm_mtx);
							list_add(list_pedidos_confirm,plato_pcb->id_pedido);
							pthread_mutex_unlock(&list_pedidos_confirm_mtx);

							agregar_cola_ready(plato_pcb);


						}//for de cantidad total - cant hecha

					}




					//respondo al cliente -- consultar en que casos se debe mandar fail
					//esta es la respuesta al cliente,
					confirmacion = OK;
				}else{//llave de validacion de error en obtener pedido
					log_info(log_config_ini,"\tSindicato devolvio error al OBTENER_PEDIDO \n");
				}
				free_struct_mensaje(rta_sindicato_RTA_OBTENER_PEDIDO, RTA_OBTENER_PEDIDO);
			}
		}

		//RESPONDO AL CLIENTE
		//OK o FAIL
		enviar_confirmacion(confirmacion,cliente_fd, RTA_CONFIRMAR_PEDIDO);
		free_struct_mensaje(id_CONFIRMAR_PEDIDO, CONFIRMAR_PEDIDO);
		free(mensaje);

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


		break;
	case CONSULTAR_PEDIDO:
		loggear_mensaje_recibido(mensaje, cod_op, log_config_ini);

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
			loggear_mensaje_enviado(cliente_rta_CONSULTAR_PEDIDO->parametros, cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje, log_config_ini);



		}else{

			//ENVIAR MJ A SINDICATO - OBTENER PEDIDO
			//1) OBTENER EL PEDIDO DEL MODULO SINDICATO
			t_nombre_y_id* sindicato_OBTENER_PEDIDO = malloc(sizeof(t_nombre_y_id));
			sindicato_OBTENER_PEDIDO->id = *id_CONSULTAR_PEDIDO;

			sindicato_OBTENER_PEDIDO->nombre.nombre = malloc(strlen(cfg_nombre_restaurante)+1);
			strcpy(sindicato_OBTENER_PEDIDO->nombre.nombre, cfg_nombre_restaurante);


			t_mensaje* mje_sindicato_OBTENER_PEDIDO = malloc(sizeof(t_mensaje));
			mje_sindicato_OBTENER_PEDIDO->tipo_mensaje = OBTENER_PEDIDO;
			mje_sindicato_OBTENER_PEDIDO->id=cfg_id;
			mje_sindicato_OBTENER_PEDIDO->parametros = sindicato_OBTENER_PEDIDO;
			enviar_mensaje(mje_sindicato_OBTENER_PEDIDO,socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);
			loggear_mensaje_enviado(mje_sindicato_OBTENER_PEDIDO->parametros, mje_sindicato_OBTENER_PEDIDO->tipo_mensaje, log_config_ini);

			free_struct_mensaje(mje_sindicato_OBTENER_PEDIDO->parametros, OBTENER_PEDIDO);
			free(mje_sindicato_OBTENER_PEDIDO);


			//RECIBIR RESPUESTA DE SINDICATO
			uint32_t error_cod_op;
			rta_obtenerPedido* rta_sindicato_RTA_OBTENER_PEDIDO = recibir_respuesta(socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO, &error_cod_op);// TODO

			liberar_conexion( socket_CONSULTAR_PEDIDO_OBTENER_PEDIDO);
			if(error_cod_op!=ERROR){
				rta_consultarPedido* rta_CONSULTAR_PEDIDO = malloc(sizeof(rta_consultarPedido));

				rta_CONSULTAR_PEDIDO->restaurante.nombre = malloc(strlen(cfg_nombre_restaurante)+1);
				strcpy(rta_CONSULTAR_PEDIDO->restaurante.nombre, cfg_nombre_restaurante);

				rta_CONSULTAR_PEDIDO->cantPlatos = rta_sindicato_RTA_OBTENER_PEDIDO->cantPedidos;
				rta_CONSULTAR_PEDIDO->platos = list_duplicate(rta_sindicato_RTA_OBTENER_PEDIDO->infoPedidos); //TODO
				rta_CONSULTAR_PEDIDO->estadoPedido=rta_sindicato_RTA_OBTENER_PEDIDO->estadoPedido;

				cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje = RTA_CONSULTAR_PEDIDO;
				cliente_rta_CONSULTAR_PEDIDO->parametros = rta_CONSULTAR_PEDIDO;
				cliente_rta_CONSULTAR_PEDIDO->id = cfg_id;
				enviar_mensaje(cliente_rta_CONSULTAR_PEDIDO,cliente_fd);
				loggear_mensaje_enviado(cliente_rta_CONSULTAR_PEDIDO->parametros, cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje, log_config_ini);

				free_struct_mensaje(cliente_rta_CONSULTAR_PEDIDO->parametros, RTA_CONSULTAR_PEDIDO);
				free(cliente_rta_CONSULTAR_PEDIDO);
				//free_struct_mensaje(rta_sindicato_RTA_OBTENER_PEDIDO,RTA_OBTENER_PEDIDO);
				free(rta_sindicato_RTA_OBTENER_PEDIDO);
			}else{
				cliente_rta_CONSULTAR_PEDIDO->id=cfg_id;
				cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje = ERROR;
				enviar_mensaje(cliente_rta_CONSULTAR_PEDIDO,cliente_fd);
				loggear_mensaje_enviado(cliente_rta_CONSULTAR_PEDIDO->parametros, cliente_rta_CONSULTAR_PEDIDO->tipo_mensaje, log_config_ini);
			}




		}

		// liberar_conexion(cliente_fd);
		break;
	case 0:
		pthread_exit(NULL);
	case -1:
		pthread_exit(NULL);
	}
	//	TODO ver que onda free_struct_mensaje(mensaje,cod_op);
}


void agregar_cola_ready(t_plato_pcb* plato){
	t_cola_afinidad* cola_afinidad;
	t_nombre* afinidad = malloc(sizeof(t_nombre));
	afinidad->nombre = string_duplicate(plato->comida.nombre);
	bool _mismo_nombre(t_cola_afinidad* cola){
		return mismo_nombre(afinidad, &(cola->afinidad));
	}
	cambiarEstado(plato, READY);
	pthread_mutex_lock(&cola_afinidades_mtx);
	cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);
	pthread_mutex_unlock(&cola_afinidades_mtx);
	free(afinidad->nombre);

	if(cola_afinidad == NULL){
		afinidad = malloc(sizeof(t_nombre));
		afinidad->nombre = "Otros";
		pthread_mutex_lock(&cola_afinidades_mtx);
		cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);
		pthread_mutex_unlock(&cola_afinidades_mtx);
	}
	free(afinidad);
	pthread_mutex_lock(&(cola_afinidad->cola_ready_mtx));
	queue_push(cola_afinidad->ready, plato);
	pthread_mutex_unlock(&(cola_afinidad->cola_ready_mtx));

	//LE AVISO AL HILO PLANIFICADOR QUE TIENE UN PEDIDO CON PLATOS A PLANIFICAR
	sem_post(&(cola_afinidad->platos_disp));
}

void planificador_ready_a_exec(t_cola_afinidad* strc_cola){
	t_plato_pcb* plato;
	t_cocinero* cocinero;
	while(true){
		sem_wait(&(strc_cola->platos_disp));
		sem_wait(&(strc_cola->cocineros_disp_sem));
		pthread_mutex_lock(&(strc_cola->cola_ready_mtx));
		plato = queue_pop(strc_cola->ready);
		pthread_mutex_unlock(&(strc_cola->cola_ready_mtx));
		cambiarEstado(plato, EXEC);
		pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
		cocinero = queue_pop(strc_cola->cola_cocineros_disp);
		pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));
		cocinero->plato_a_cocinar = plato;

		pthread_mutex_lock(&mutex_EXEC);
		list_add(platos_EXEC, plato);
		pthread_mutex_unlock(&mutex_EXEC);

		t_paso* paso_siguiente = list_get(cocinero->plato_a_cocinar->pasos, 0);
		log_debug(log_oficial, "[INICIO_OPERACION]: El plato %s con ID: %d comenzó a %s", cocinero->plato_a_cocinar->comida.nombre,
				cocinero->plato_a_cocinar->id_plato, paso_siguiente->paso.nombre);

		pthread_mutex_unlock(&(cocinero->mtx_exec));
	}
}

void planificador_exec(t_cola_afinidad* strc_cola){
	t_cocinero* cocinero;
	t_paso* paso_siguiente;
	pthread_t hilo;

	uint32_t id_plato_actual;

	bool _mismo_id(t_plato_pcb* id_plato){
		return mismo_id(id_plato->id_plato, id_plato_actual);
	}

	while(true){
		sem_wait(&(strc_cola->sem_exec));
		pthread_mutex_lock(&(strc_cola->cola_cocineros_exec_mtx));
		cocinero = queue_pop(strc_cola->cola_cocineros_exec);
		pthread_mutex_unlock(&(strc_cola->cola_cocineros_exec_mtx));

		id_plato_actual = cocinero->plato_a_cocinar->id_plato;

		paso_siguiente = list_get(cocinero->plato_a_cocinar->pasos, 0);
		if(paso_siguiente->duracion == 0){ //si el paso ya termino

			log_debug(log_oficial, "[FINALIZACION_OPERACION]: El plato %s con ID: %d terminó de %s", cocinero->plato_a_cocinar->comida.nombre,
					cocinero->plato_a_cocinar->id_plato, paso_siguiente->paso.nombre);

			list_remove_and_destroy_element(cocinero->plato_a_cocinar->pasos,0, (void*)free_pasos);
			if(list_is_empty(cocinero->plato_a_cocinar->pasos)){


				pthread_mutex_lock(&mutex_EXEC);
				t_plato_pcb* plato_a_liberar = list_remove_by_condition(platos_EXEC, (void*)_mismo_id);
				pthread_mutex_unlock(&mutex_EXEC);

				pthread_create(&hilo, NULL,(void*)terminar_plato, (void*)plato_a_liberar);
				list_add(hilos_reposo, &hilo);

				cocinero->plato_a_cocinar = NULL;
				cocinero->ciclos_ejecutando = 0;

				pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
				queue_push(strc_cola->cola_cocineros_disp, cocinero);
				pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));
				sem_post(&(strc_cola->cocineros_disp_sem));
			}else{
				paso_siguiente = list_get(cocinero->plato_a_cocinar->pasos, 0);
				if(string_equals_ignore_case(paso_siguiente->paso.nombre, REPOSAR)){ //si tiene que reposar
					cambiarEstado(cocinero->plato_a_cocinar, BLOCK);

					log_debug(log_oficial, "[INICIO_OPERACION]: El plato %s con ID: %d comenzó a %s", cocinero->plato_a_cocinar->comida.nombre,
							cocinero->plato_a_cocinar->id_plato, paso_siguiente->paso.nombre);

					pthread_mutex_lock(&mutex_EXEC);
					list_remove_by_condition(platos_EXEC, (void*)_mismo_id);
					pthread_mutex_unlock(&mutex_EXEC);

					pthread_mutex_lock(&mutex_REPOSANDO);
					list_add(platos_REPOSANDO, cocinero->plato_a_cocinar);
					pthread_mutex_unlock(&mutex_REPOSANDO);

					pthread_create(&hilo, NULL,(void*)reposar_plato, (void*)cocinero->plato_a_cocinar);
					list_add(hilos_reposo, &hilo);
					cocinero->plato_a_cocinar = NULL;
					cocinero->ciclos_ejecutando = 0;

					pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
					queue_push(strc_cola->cola_cocineros_disp, cocinero);
					pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));
					sem_post(&(strc_cola->cocineros_disp_sem));
				}else if(string_equals_ignore_case(paso_siguiente->paso.nombre, HORNEAR)){ //si tiene que hornear
					cambiarEstado(cocinero->plato_a_cocinar, BLOCK);

					log_debug(log_oficial, "[INICIO_OPERACION]: El plato %s con ID: %d comenzó a %s", cocinero->plato_a_cocinar->comida.nombre,
							cocinero->plato_a_cocinar->id_plato, paso_siguiente->paso.nombre);

					pthread_mutex_lock(&ready_hornos_mtx);
					queue_push(ready_hornos, cocinero->plato_a_cocinar);
					pthread_mutex_unlock(&ready_hornos_mtx);
					sem_post(&sem_ready_hornos);
					cocinero->plato_a_cocinar = NULL;
					cocinero->ciclos_ejecutando = 0;

					pthread_mutex_lock(&mutex_EXEC);
					list_remove_by_condition(platos_EXEC, (void*)_mismo_id);
					pthread_mutex_unlock(&mutex_EXEC);

					pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
					queue_push(strc_cola->cola_cocineros_disp, cocinero);
					pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));
					sem_post(&(strc_cola->cocineros_disp_sem));
				}else{
					cambiarEstado(cocinero->plato_a_cocinar, READY); //si tiene que volver a ready

					//TODO ver que se loguea

					pthread_mutex_lock(&(strc_cola->cola_ready_mtx));
					queue_push(strc_cola->ready, cocinero->plato_a_cocinar);
					pthread_mutex_unlock(&(strc_cola->cola_ready_mtx));
					cocinero->plato_a_cocinar = NULL;
					cocinero->ciclos_ejecutando = 0;
					pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
					queue_push(strc_cola->cola_cocineros_disp, cocinero);
					pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));

					pthread_mutex_lock(&mutex_EXEC);
					list_remove_by_condition(platos_EXEC, (void*)_mismo_id);
					pthread_mutex_unlock(&mutex_EXEC);

					sem_post(&(strc_cola->cocineros_disp_sem));
					sem_post(&(strc_cola->platos_disp));
				}
			}
		}else{ //si sigue ejecutando el mismo paso
			if(string_equals_ignore_case(cfg_algoritmo_planificacion, RR) && cocinero->ciclos_ejecutando == cfg_quantum){
				cambiarEstado(cocinero->plato_a_cocinar, READY); //si tiene que volver a ready por fin de quantum
				pthread_mutex_lock(&(strc_cola->cola_ready_mtx));
				queue_push(strc_cola->ready, cocinero->plato_a_cocinar);
				pthread_mutex_unlock(&(strc_cola->cola_ready_mtx));
				cocinero->plato_a_cocinar = NULL;
				cocinero->ciclos_ejecutando = 0;
				pthread_mutex_lock(&(strc_cola->cola_cocineros_disp_mtx));
				queue_push(strc_cola->cola_cocineros_disp, cocinero);
				pthread_mutex_unlock(&(strc_cola->cola_cocineros_disp_mtx));

				pthread_mutex_lock(&mutex_EXEC);
				list_remove_by_condition(platos_EXEC, (void*)_mismo_id);
				pthread_mutex_unlock(&mutex_EXEC);

				sem_post(&(strc_cola->cocineros_disp_sem));
				sem_post(&(strc_cola->platos_disp));
			}else{
				pthread_mutex_unlock(&(cocinero->mtx_exec));
			}
		}
	}
}


void terminar_plato(t_plato_pcb* plato){
	int socket_app_plato_listo = iniciar_cliente(cfg_ip_app, cfg_puerto_app);
	int socket_sindicato_plato_listo = iniciar_cliente(cfg_ip_sindicato, cfg_puerto_sindicato);
	//ENVIO A APP PLATO_LISTO
	if(socket_app_plato_listo == -1){
		//TODO error
	}else{
		uint32_t cod_op;
		t_mensaje* mensaje = malloc(sizeof(t_mensaje));
		mensaje->id = cfg_id;
		mensaje->tipo_mensaje = PLATO_LISTO;
		m_platoListo * plato_listo = malloc(sizeof(m_platoListo));
		plato_listo->comida.nombre = string_duplicate(plato->comida.nombre);
		plato_listo->restaurante.nombre = string_duplicate(cfg_nombre_restaurante);
		plato_listo->idPedido = plato->id_pedido;
		mensaje->parametros = plato_listo;
		enviar_mensaje(mensaje, socket_app_plato_listo);
		loggear_mensaje_enviado(mensaje->parametros, mensaje->tipo_mensaje, log_config_ini);

		free_struct_mensaje(mensaje->parametros, PLATO_LISTO);
		free(mensaje);
		uint32_t* rta_plato_listo = recibir_respuesta(socket_app_plato_listo, &cod_op);
		if(cod_op == RTA_PLATO_LISTO){
			loggear_mensaje_recibido(rta_plato_listo, cod_op, log_config_ini);
			free_struct_mensaje(rta_plato_listo, cod_op);
		}else{
			//error TODO
		}
		liberar_conexion(socket_app_plato_listo);

	}

	//ENVIO A SINDICATO PLATO_LISTO
	if(socket_sindicato_plato_listo == -1){
		//TODO error
	}else{
		uint32_t cod_op;
		t_mensaje* mensaje = malloc(sizeof(t_mensaje));
		mensaje->id = cfg_id;
		mensaje->tipo_mensaje = PLATO_LISTO;
		m_platoListo * plato_listo = malloc(sizeof(m_platoListo));
		plato_listo->comida.nombre = string_duplicate(plato->comida.nombre);
		plato_listo->restaurante.nombre = string_duplicate(cfg_nombre_restaurante);
		plato_listo->idPedido = plato->id_pedido;
		mensaje->parametros = plato_listo;
		enviar_mensaje(mensaje, socket_sindicato_plato_listo);
		loggear_mensaje_enviado(mensaje->parametros, mensaje->tipo_mensaje, log_config_ini);

		free_struct_mensaje(mensaje->parametros, PLATO_LISTO);
		free(mensaje);
		uint32_t* rta_plato_listo = recibir_respuesta(socket_sindicato_plato_listo, &cod_op);
		if(cod_op == RTA_PLATO_LISTO){
			loggear_mensaje_recibido(rta_plato_listo, cod_op, log_config_ini);
			free_struct_mensaje(rta_plato_listo, cod_op);
		}else{
			//error TODO
		}
		liberar_conexion(socket_sindicato_plato_listo);

	}

	log_debug(log_oficial, "[PLATO_LISTO]: Finalizó el plato %s con ID: %d", plato->comida.nombre, plato->id_plato);
	free_pcb_plato(plato);


}

void free_pcb_plato(t_plato_pcb* plato){
	//	list_destroy_and_destroy_elements(plato->pasos, (void*) free_pasos);
	free(plato->comida.nombre);
	free(plato);
}

void reposar_plato(t_plato_pcb* plato){
	t_paso* paso = list_remove(plato->pasos, 0);

	bool _mismo_id(t_plato_pcb* id){
		return mismo_id(id->id_plato, plato->id_plato);
	}


	for(int i = 0; i< paso->duracion; i ++){
		pthread_mutex_trylock(&plato->mutex_clock);
		pthread_mutex_lock(&plato->mutex_clock);
	}
	free_pasos(paso);
	t_paso* paso_siguiente = list_get(plato->pasos, 0);

	log_debug(log_oficial, "[FINALIZACION_OPERACION]: El plato %s con ID: %d terminó de REPOSAR", plato->comida.nombre,
			plato->id_plato);

	if(string_equals_ignore_case(paso_siguiente->paso.nombre, HORNEAR)){ //no cambia de estado porque ya estaba bloqueado

		log_debug(log_oficial, "[INICIO_OPERACION]: El plato %s con ID: %d comenzó a %s", plato->comida.nombre,
				plato->id_plato, paso_siguiente->paso.nombre);

		pthread_mutex_lock(&ready_hornos_mtx);
		queue_push(ready_hornos, plato);
		pthread_mutex_unlock(&ready_hornos_mtx);
		sem_post(&sem_ready_hornos);

		pthread_mutex_lock(&mutex_REPOSANDO);
		list_remove_by_condition(platos_REPOSANDO, (void*)_mismo_id);
		pthread_mutex_unlock(&mutex_REPOSANDO);

	}else{

		pthread_mutex_lock(&mutex_REPOSANDO);
		list_remove_by_condition(platos_REPOSANDO, (void*)_mismo_id);
		pthread_mutex_unlock(&mutex_REPOSANDO);

		agregar_cola_ready(plato);
	}
}

void planificar_hornos(){
	t_plato_pcb* plato_a_hornear;
	t_horno* horno;
	while(true){
		sem_wait(&sem_ready_hornos);
		sem_wait(&hornos_disp_sem);
		pthread_mutex_lock(&ready_hornos_mtx);
		plato_a_hornear = queue_pop(ready_hornos);
		pthread_mutex_unlock(&ready_hornos_mtx);
		pthread_mutex_lock(&hornos_disp_mtx);
		horno = queue_pop(hornos_disp);
		pthread_mutex_unlock(&hornos_disp_mtx);
		horno->plato_a_cocinar = plato_a_hornear;
		pthread_mutex_unlock(&(horno->mtx_IO));

		pthread_mutex_lock(&mutex_HORNEANDO);
		list_add(platos_HORNEANDO, plato_a_hornear);
		pthread_mutex_unlock(&mutex_HORNEANDO);
	}
}

void hornear(t_horno* horno){
	t_paso* paso;

	uint32_t id_plato_actual;


	bool _mismo_id(t_plato_pcb* id){
		return mismo_id(id->id_plato, id_plato_actual);
	}

	while(true){
		pthread_mutex_lock(&(horno->mtx_IO));
		paso = list_remove(horno->plato_a_cocinar->pasos, 0);
		for(int i = 0; i< paso->duracion; i ++){
			pthread_mutex_trylock(&horno->plato_a_cocinar->mutex_clock);
			pthread_mutex_lock(&horno->plato_a_cocinar->mutex_clock);
		}
		free_pasos(paso);

		log_debug(log_oficial, "[FINALIZACION_OPERACION]: El plato %s con ID: %d terminó de HORNEAR", horno->plato_a_cocinar->comida.nombre,
				horno->plato_a_cocinar->id_plato);
		t_paso* paso_siguiente = list_get(horno->plato_a_cocinar->pasos, 0);


		id_plato_actual = horno->plato_a_cocinar->id_plato;

		pthread_mutex_lock(&mutex_HORNEANDO);
		list_remove_by_condition(platos_HORNEANDO, (void*)_mismo_id);
		pthread_mutex_unlock(&mutex_HORNEANDO);


		if(string_equals_ignore_case(paso_siguiente->paso.nombre, REPOSAR)){ //si tiene que reposar
			log_debug(log_oficial, "[INICIO_OPERACION]: El plato %s con ID: %d comenzó a %s", horno->plato_a_cocinar->comida.nombre,
					horno->plato_a_cocinar->id_plato, paso_siguiente->paso.nombre);

			pthread_mutex_lock(&mutex_REPOSANDO);
			list_add(platos_REPOSANDO, horno->plato_a_cocinar);
			pthread_mutex_unlock(&mutex_REPOSANDO);
			pthread_t hilo;
			pthread_create(&hilo, NULL,(void*)reposar_plato, (void*)horno->plato_a_cocinar);
			list_add(hilos_reposo, &hilo);
		}else{
			agregar_cola_ready(horno->plato_a_cocinar);
		}
		horno->plato_a_cocinar = NULL;
		pthread_mutex_lock(&hornos_disp_mtx);
		queue_push(hornos_disp, horno);
		pthread_mutex_unlock(&hornos_disp_mtx);
		sem_post(&hornos_disp_sem);
	}
}

void cocinar(t_cocinero* cocinero){
	t_cola_afinidad* cola_afinidad;
	t_nombre* afinidad;
	bool _mismo_nombre(t_cola_afinidad* cola){
		return mismo_nombre(afinidad, &(cola->afinidad));
	}
	while(1){
		pthread_mutex_lock(&(cocinero->mtx_exec));
		pthread_mutex_trylock(&cocinero->plato_a_cocinar->mutex_clock);
		pthread_mutex_lock(&cocinero->plato_a_cocinar->mutex_clock);

		t_paso* paso_siguiente = list_get(cocinero->plato_a_cocinar->pasos, 0);
		paso_siguiente->duracion --;
		cocinero->ciclos_ejecutando ++;
		afinidad = malloc(sizeof(t_nombre));
		afinidad->nombre = string_duplicate(cocinero->plato_a_cocinar->comida.nombre);

		pthread_mutex_lock(&cola_afinidades_mtx);
		cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);

		free(afinidad->nombre);
		if(cola_afinidad == NULL){
			afinidad->nombre = "Otros"; //TODO define de otros gg
			cola_afinidad = list_find(colas_afinidades, (void*)_mismo_nombre);
		}

		free(afinidad);
		pthread_mutex_unlock(&cola_afinidades_mtx);
		pthread_mutex_lock(&(cola_afinidad->cola_cocineros_exec_mtx));
		queue_push(cola_afinidad->cola_cocineros_exec, cocinero);
		pthread_mutex_unlock(&(cola_afinidad->cola_cocineros_exec_mtx));
		sem_post(&(cola_afinidad->sem_exec));
	}
}

void cambiarEstado (t_plato_pcb* plato, est_planif nuevoEstado){
	if(plato->estado == nuevoEstado) return;
	if(cambioEstadoValido(plato->estado, nuevoEstado)){
		plato->estado = nuevoEstado;
		log_info(log_config_ini, "Se cambio al plato %s a la cola %s", plato->comida.nombre, stringEstado(plato->estado));
	}else{
		printf("Estado invalido");
	}
}

bool cambioEstadoValido(est_planif estadoViejo,est_planif nuevoEstado){
	switch (estadoViejo){
	case NEW:
		if(nuevoEstado == READY) return true;
		else return false;
		break;
	case READY:
		if(nuevoEstado == EXEC) return true;
		else return false;
		break;
	case EXEC:
		if(nuevoEstado == READY || nuevoEstado == BLOCK || nuevoEstado == EXIT) return true;
		else return false;
		break;
	case BLOCK:
		if(nuevoEstado == READY) return true;
		else return false;
		break;
	case EXIT:
		return false;
		break;
	}
	return false;
}

char* stringEstado(est_planif estado){
	switch (estado){
	case NEW:
		return "NEW";
		break;
	case READY:
		return "READY";
		break;
	case EXEC:
		return "EXEC";
		break;
	case BLOCK:
		return "BLOCK";
		break;
	case EXIT:
		return "EXIT";
		break;
	default:
		return "";
		break;
	}
}

void enviar_confirmacion(t_confirmacion _confirmacion, int cliente, op_code cod_op){
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

void* recibir_respuesta(int socket, uint32_t* cod_op){ //TODO cambiar en las demas llamadas  poner cod op en -1?
	//	op_code cod_op = 999;
	uint32_t id_proceso;
	void* mensaje=NULL;
	int error = 0;
	int _recv = recv(socket, cod_op, sizeof(op_code), MSG_WAITALL);
	log_info(log_config_ini, "cod op: %d\n", *cod_op);

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

	mensaje = deserializar_mensaje(buffer, *cod_op);
	loggear_mensaje_recibido(mensaje, *cod_op, log_config_ini);
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

//CLOCK

void fhilo_clock() {
	unsigned ciclo_display_counter = 0;
	while (true) {
		pthread_mutex_lock(&mutex_EXEC);
		pthread_mutex_lock(&mutex_REPOSANDO);
		pthread_mutex_lock(&mutex_HORNEANDO);
		//		log_debug(
		//				log_config_ini,
		//				"[CLOCK]: -------------------------------------------------------- Ciclo %i",
		//				++ciclo_display_counter
		//		);
		for (unsigned index_EXEC = 0; index_EXEC < list_size(platos_EXEC); index_EXEC++) {
			pthread_mutex_unlock(&((t_plato_pcb*)list_get(platos_EXEC, index_EXEC))->mutex_clock);
		}
		for (unsigned index_REPOSANDO = 0; index_REPOSANDO < list_size(platos_REPOSANDO); index_REPOSANDO++) {
			pthread_mutex_unlock(&((t_plato_pcb*)list_get(platos_REPOSANDO, index_REPOSANDO))->mutex_clock);
		}
		for (unsigned index_HORNEANDO = 0; index_HORNEANDO < list_size(platos_HORNEANDO); index_HORNEANDO++) {
			pthread_mutex_unlock(&((t_plato_pcb*)list_get(platos_HORNEANDO, index_HORNEANDO))->mutex_clock);
		}
		pthread_mutex_unlock(&mutex_HORNEANDO);
		pthread_mutex_unlock(&mutex_REPOSANDO);
		pthread_mutex_unlock(&mutex_EXEC);
		sleep(cfg_retardo_ciclo_CPU);
	}
}

