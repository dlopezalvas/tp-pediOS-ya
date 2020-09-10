/*
 * utils_restaurante.c
 *
 *  Created on: 1 sep. 2020
 *      Author: utnso
 */

#include "utils_restaurante.h"

void iniciar_restaurante(char* ip, int puerto){
	int estado_conexion=-1;
	int conexion_sindicato;
	int tiempo_reconexion=10;

	log_info(log_config_ini, "el estado de conexion es: %d \n",estado_conexion);



	while(estado_conexion==-1){
	        //(LOG) PTO 10

	        conexion_sindicato = iniciar_cliente(ip,puerto);
	        log_info(log_config_ini, "Conectar con sindicato \n", conexion_sindicato);

			estado_conexion= conexion_sindicato;
			if(estado_conexion==-1){
	            //(LOG) PTO 11

				log_info(log_config_ini, "intento de reconexion en %i segundos \n", tiempo_reconexion);

				delay(tiempo_reconexion);
			}else{
	        //(LOG) PTO 11
	        log_info(log_config_ini,"La reconexión con Sindicato fue exitosa \n");
	        estado_conexion=1;



				}

			if(conexion_sindicato==-1){
				log_info(log_config_ini, "intento de reconexion en %i segundos \n", tiempo_reconexion);
			}else{


				//Enviar mje obtener restaurante

				t_mensaje* tipomje = malloc(sizeof(t_mensaje));
				tipomje->tipo_mensaje=OBTENER_RESTAURANTE;

				t_nombre* nombre_restaurante=malloc(sizeof(t_nombre));
				nombre_restaurante->nombre="uggis";

				tipomje->parametros=nombre_restaurante;

				//enviar_mensaje(tipomje, conexion_sindicato);





				//Recibir respuesta obtener restaurante
				rta_obtenerRestaurante* metadata_rest = malloc(sizeof(rta_obtenerRestaurante));

				//metadata_rest = metadata_restaurante(conexion_sindicato);



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

				log_info(log_config_ini, "termino la carga de los datos del sindicato \n");


				//cargo los datos de prueba

				metadata_rest->cantCocineroAfinidad=cantidad_cocineros;
				log_info(log_config_ini, "cantidad de cocineros %d\n",metadata_rest->cantCocineroAfinidad);

				metadata_rest->cocineroAfinidad = list_create();
				list_add(metadata_rest->cocineroAfinidad, afinidad1);
				char* afi1= list_get(metadata_rest->cocineroAfinidad, 0);
				log_info(log_config_ini, "afinidad1: %s",afi1);

				list_add(metadata_rest->cocineroAfinidad, afinidad2);
				char* afi2= list_get(metadata_rest->cocineroAfinidad, 1);
				log_info(log_config_ini, "afinidad2: %s",afi2);

				metadata_rest->posicion.x=posx;
				metadata_rest->posicion.y=posy;
				log_info(log_config_ini, "coordenadas x:%d ,  y:%d ",metadata_rest->posicion.x,metadata_rest->posicion.y);


				metadata_rest->cantRecetas=cant_recetas;
				log_info(log_config_ini, "cantiad de recetas: %d",metadata_rest->cantRecetas);

				list_add(metadata_rest->recetas, receta1);
				log_info(log_config_ini, "receta1: %s",list_get(metadata_rest->cocineroAfinidad, 0));

				list_add(metadata_rest->recetas, receta2);
				log_info(log_config_ini, "receta2: %s",list_get(metadata_rest->cocineroAfinidad, 1));

				list_add(metadata_rest->recetas, receta3);
				log_info(log_config_ini, "receta3: %s",list_get(metadata_rest->cocineroAfinidad, 2));

				metadata_rest->cantHornos=2;

				log_info(log_config_ini, "Datos de restaurante cargados \n");
				log_info(log_config_ini, "cantidad de cocineros %d  \n", metadata_rest->cantCocineroAfinidad);
				log_info(log_config_ini, "posx %d  \n", metadata_rest->posicion.x);
				log_info(log_config_ini, "posy %d  \n", metadata_rest->posicion.y);
				log_info(log_config_ini, "cantidad de recetas %d  \n", metadata_rest->cantRecetas);
				log_info(log_config_ini, "cantidad de hornos %d  \n", metadata_rest->cantHornos);



			}
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

