#include "serializacion.h"

void enviar_mensaje(t_mensaje* mensaje, int socket){

	t_paquete* paquete = malloc(sizeof(t_paquete));

	t_buffer* buffer_cargado = cargar_buffer(mensaje);

	paquete -> buffer = buffer_cargado;

	paquete -> codigo_operacion = mensaje -> tipo_mensaje;

	int bytes = 0;

	void* a_enviar = serializar_paquete(paquete, &bytes);

	send(socket,a_enviar,bytes,0);

	free(a_enviar);
	free(paquete -> buffer->stream);
	free(paquete->buffer);
	free(paquete);

}

void* serializar_paquete(t_paquete* paquete, int *bytes){

	int size = sizeof(uint32_t) + paquete->buffer->size + sizeof(op_code);

	void* a_enviar = malloc (size);

	memcpy(a_enviar + *bytes, &paquete-> codigo_operacion, sizeof(paquete->codigo_operacion));
	*bytes += sizeof(paquete->codigo_operacion);
	memcpy(a_enviar  + *bytes, &(paquete -> buffer -> size),sizeof(int));
	*bytes += sizeof(int);
	memcpy(a_enviar  + *bytes, paquete -> buffer -> stream, paquete -> buffer -> size);
	*bytes += paquete->buffer->size;

	return a_enviar;
}

t_buffer* cargar_buffer(t_mensaje* mensaje){

	uint32_t proceso = mensaje -> tipo_mensaje;
	void* parametros = mensaje -> parametros;

	switch(proceso){
	case(CONSULTAR_RESTAURANTES): return buffer_vacio();
	case(RTA_CONSULTAR_RESTAURANTES): return buffer_rta_consultar_restaurantes(parametros);
	case(SELECCIONAR_RESTAURANTE): return buffer_seleccionar_restaurante(parametros);
	case(RTA_SELECCIONAR_RESTAURANTE): return buffer_id_o_confirmacion(parametros);
	case(OBTENER_RESTAURANTE): return buffer_nombre_restaurante(parametros);
	case(RTA_OBTENER_RESTAURANTE): return buffer_rta_obtener_restaurante(parametros); //TODO lista
	case(CONSULTAR_PLATOS): return buffer_nombre_restaurante(parametros);
	case(RTA_CONSULTAR_PLATOS): return buffer_rta_consultar_platos(parametros); //TODO lista
	case(CREAR_PEDIDO): return buffer_vacio();
	case(RTA_CREAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(GUARDAR_PEDIDO): return buffer_nombre_y_id(parametros);
	case(RTA_GUARDAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(AGREGAR_PLATO): return buffer_nombre_y_id(parametros);
	case(RTA_AGREGAR_PLATO): return buffer_id_o_confirmacion(parametros);
	case(GUARDAR_PLATO): return buffer_guardar_plato(parametros);
	case(RTA_GUARDAR_PLATO): return buffer_id_o_confirmacion(parametros);
	case(CONFIRMAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(RTA_CONFIRMAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(PLATO_LISTO): return buffer_plato_listo(parametros);
	case(RTA_PLATO_LISTO): return buffer_id_o_confirmacion(parametros);
	case(CONSULTAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(RTA_CONSULTAR_PEDIDO): return buffer_rta_consultar_pedido(parametros); //TODO lista
	case(OBTENER_PEDIDO): return buffer_nombre_y_id(parametros);
	case(RTA_OBTENER_PEDIDO): return buffer_rta_obtener_pedido(parametros); //TODO lista
	case(FINALIZAR_PEDIDO): return buffer_nombre_y_id(parametros);
	case(RTA_FINALIZAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	case(TERMINAR_PEDIDO): return buffer_nombre_y_id(parametros);
	case(RTA_TERMINAR_PEDIDO): return buffer_id_o_confirmacion(parametros);
	}
	return 0;
}

// Serializacion


//seleccionar_restaurante

t_buffer* buffer_seleccionar_restaurante(m_seleccionarRestaurante* seleccionarRestaurante){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	seleccionarRestaurante->restaurante.largo_nombre = strlen(seleccionarRestaurante->restaurante.nombre);


	buffer -> size = sizeof(uint32_t)*2 + seleccionarRestaurante->restaurante.largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &seleccionarRestaurante->cliente, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &seleccionarRestaurante->restaurante.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, seleccionarRestaurante->restaurante.nombre, seleccionarRestaurante->restaurante.largo_nombre);

	buffer -> stream = stream;

	return buffer;
}

m_seleccionarRestaurante* deserializar_seleccionar_restaurante(void* buffer){

	m_seleccionarRestaurante* seleccionarRestaurante = malloc(sizeof(m_seleccionarRestaurante));
	memcpy(&seleccionarRestaurante->cliente, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&seleccionarRestaurante->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	seleccionarRestaurante->restaurante.nombre = malloc(seleccionarRestaurante->restaurante.largo_nombre + 1);
	memcpy(seleccionarRestaurante->restaurante.nombre, buffer, seleccionarRestaurante->restaurante.largo_nombre);
	seleccionarRestaurante->restaurante.nombre[seleccionarRestaurante->restaurante.largo_nombre] = '\0';

	return seleccionarRestaurante;
}

//confirmacion y id pedido: RTA_SELECCIONAR_RESTAURANTE - RTA_CREAR_PEDIDO - RTA_GUARDAR_PEDIDO - RTA_AGREGAR_PLATO - RTA_GUARDAR_PLATO - CONFIRMAR_PEDIDO -
//RTA_CONFIRMAR_PEDIDO - RTA_PLATO_LISTO - CONSULTAR_PEDIDO - RTA_FINALIZAR_PEDIDO - RTA_TERMINAR_PEDIDO

t_buffer* buffer_id_o_confirmacion(uint32_t* numero){
	t_buffer* buffer = malloc(sizeof(t_buffer));


	buffer -> size = sizeof(uint32_t);
	void* stream = malloc(buffer -> size);

	memcpy(stream, &numero, sizeof(uint32_t));

	buffer -> stream = stream;

	return buffer;
}

uint32_t* deserializar_id_o_confirmacion(void* buffer){

	uint32_t* numero = malloc(sizeof(uint32_t));
	memcpy(&numero, buffer, sizeof(uint32_t));

	return numero;
}

//nombre restaurante: OBTENER_RESTAURANTE - CONSULTAR_PLATOS


t_buffer* buffer_nombre_restaurante(t_nombre* nombre_restaurante){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	nombre_restaurante->largo_nombre = strlen(nombre_restaurante->nombre);

	buffer -> size = sizeof(uint32_t) + nombre_restaurante->largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &nombre_restaurante->largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombre_restaurante->nombre, nombre_restaurante->largo_nombre);

	buffer -> stream = stream;

	return buffer;
}

t_nombre* deserealizar_nombre_restaurante(void* buffer){
	t_nombre* nombre_restaurante = malloc(sizeof(t_nombre));

	memcpy(&nombre_restaurante->largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	nombre_restaurante->nombre = malloc(nombre_restaurante->largo_nombre + 1);
	memcpy(nombre_restaurante->nombre, buffer, nombre_restaurante->largo_nombre);
	nombre_restaurante->nombre[nombre_restaurante->largo_nombre] = '\0';

	return nombre_restaurante;
}


//id y nombre: GUARDAR_PEDIDO - AGREGAR_PLATO - OBTENER_PEDIDO - FINALIZAR_PEDIDO - TERMINAR_PEDIDO

t_buffer* buffer_nombre_y_id(t_nombre_y_id* nombre_y_id){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	nombre_y_id->nombre.largo_nombre = strlen(nombre_y_id->nombre.nombre);


	buffer -> size = sizeof(uint32_t)*2 + nombre_y_id->nombre.largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &nombre_y_id->id, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &nombre_y_id->nombre.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombre_y_id->nombre.nombre, nombre_y_id->nombre.largo_nombre);

	buffer -> stream = stream;

	return buffer;
}

t_nombre_y_id* deserializar_nombre_y_id(void* buffer){

	t_nombre_y_id* nombre_y_id = malloc(sizeof(m_seleccionarRestaurante));
	memcpy(&nombre_y_id->id, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&nombre_y_id->nombre.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	nombre_y_id->nombre.nombre = malloc(nombre_y_id->nombre.largo_nombre + 1);
	memcpy(nombre_y_id->nombre.nombre, buffer, nombre_y_id->nombre.largo_nombre);
	nombre_y_id->nombre.nombre[nombre_y_id->nombre.largo_nombre] = '\0';

	return nombre_y_id;
}


//guardar plato

t_buffer* buffer_guardar_plato(m_guardarPlato* guardar_plato){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	guardar_plato->comida.largo_nombre = strlen(guardar_plato->comida.nombre);
	guardar_plato->restaurante.largo_nombre = strlen(guardar_plato->restaurante.nombre);

	buffer -> size = sizeof(uint32_t)*4 + guardar_plato->comida.largo_nombre + guardar_plato->restaurante.largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &guardar_plato->idPedido, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &guardar_plato->comida.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, guardar_plato->comida.nombre, guardar_plato->comida.largo_nombre);
	offset += guardar_plato->comida.largo_nombre;
	memcpy(stream + offset, &guardar_plato->cantidad, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &guardar_plato->restaurante.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, guardar_plato->restaurante.nombre, guardar_plato->restaurante.largo_nombre);

	buffer -> stream = stream;

	return buffer;
}


m_guardarPlato* deserializar_guardar_plato(void* buffer){

	m_guardarPlato* guardar_plato = malloc(sizeof(m_guardarPlato));
	memcpy(&guardar_plato->idPedido, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	memcpy(&guardar_plato->comida.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	guardar_plato->comida.nombre = malloc(guardar_plato->comida.largo_nombre + 1);
	memcpy(guardar_plato->comida.nombre, buffer, guardar_plato->comida.largo_nombre);
	buffer += guardar_plato->comida.largo_nombre;
	guardar_plato->comida.nombre[guardar_plato->comida.largo_nombre] = '\0';

	memcpy(&guardar_plato->cantidad, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	memcpy(&guardar_plato->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	guardar_plato->restaurante.nombre = malloc(guardar_plato->restaurante.largo_nombre + 1);
	memcpy(guardar_plato->restaurante.nombre, buffer, guardar_plato->restaurante.largo_nombre);
	guardar_plato->restaurante.nombre[guardar_plato->restaurante.largo_nombre] = '\0';

	return guardar_plato;
}

//plato_listo

t_buffer* buffer_plato_listo(m_platoListo* plato_listo){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	plato_listo->comida.largo_nombre = strlen(plato_listo->comida.nombre);
	plato_listo->restaurante.largo_nombre = strlen(plato_listo->restaurante.nombre);

	buffer -> size = sizeof(uint32_t)*3 + plato_listo->comida.largo_nombre + plato_listo->restaurante.largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &plato_listo->idPedido, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &plato_listo->comida.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, plato_listo->comida.nombre, plato_listo->comida.largo_nombre);
	offset += plato_listo->comida.largo_nombre;
	memcpy(stream + offset, &plato_listo->restaurante.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, plato_listo->restaurante.nombre, plato_listo->restaurante.largo_nombre);

	buffer -> stream = stream;

	return buffer;
}


m_platoListo* deserializar_plato_listo(void* buffer){

	m_platoListo* plato_listo = malloc(sizeof(m_guardarPlato));
	memcpy(&plato_listo->idPedido, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	memcpy(&plato_listo->comida.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	plato_listo->comida.nombre = malloc(plato_listo->comida.largo_nombre + 1);
	memcpy(plato_listo->comida.nombre, buffer, plato_listo->comida.largo_nombre);
	buffer += plato_listo->comida.largo_nombre;
	plato_listo->comida.nombre[plato_listo->comida.largo_nombre] = '\0';

	memcpy(&plato_listo->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	plato_listo->restaurante.nombre = malloc(plato_listo->restaurante.largo_nombre + 1);
	memcpy(plato_listo->restaurante.nombre, buffer, plato_listo->restaurante.largo_nombre);
	plato_listo->restaurante.nombre[plato_listo->restaurante.largo_nombre] = '\0';

	return plato_listo;
}

//buffer vacio: CONSULTAR_RESTAURANTES - CREAR_PEDIDO

t_buffer* buffer_vacio(){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	return buffer;
}

//rta consultar restaurantes
t_buffer* buffer_rta_consultar_restaurantes(rta_consultarRestaurantes* lista_restaurantes){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	lista_restaurantes->cantRestaurantes = lista_restaurantes->restaurantes->elements_count;

	int size_strings = tamanio_lista_strings(lista_restaurantes->restaurantes);

	buffer -> size = sizeof(uint32_t) + size_strings;

	void* stream = malloc(buffer -> size);
	int offset = 0;

	memcpy(stream + offset, &lista_restaurantes->cantRestaurantes, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_nombre* nombre_restaurante;

	for(int i = 0; i < lista_restaurantes->cantRestaurantes; i++){
		nombre_restaurante->nombre = list_get(lista_restaurantes->restaurantes, i);
		nombre_restaurante->largo_nombre = strlen(nombre_restaurante->nombre);
		memcpy(stream + offset, &nombre_restaurante->largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, nombre_restaurante->nombre, nombre_restaurante->largo_nombre);
		offset += nombre_restaurante->largo_nombre;
	}

	buffer -> stream = stream;
	return buffer;
}

rta_consultarRestaurantes* deserializar_rta_consultarRestaurantes(void* buffer){

	rta_consultarRestaurantes* lista_restaurantes = malloc(sizeof(rta_consultarRestaurantes));

	lista_restaurantes->restaurantes = list_create();

	memcpy(&lista_restaurantes->cantRestaurantes, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_nombre* nombre_restaurante;

	for(int i = 0; i < lista_restaurantes->cantRestaurantes; i++){
		memcpy(&nombre_restaurante->largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		nombre_restaurante = malloc(nombre_restaurante->largo_nombre);
		memcpy(nombre_restaurante->nombre, buffer, nombre_restaurante->largo_nombre);
		buffer += nombre_restaurante->largo_nombre;
		list_add(lista_restaurantes, nombre_restaurante);
	}

	return lista_restaurantes;
}


//rta obtener restaurante

t_buffer* buffer_rta_obtener_restaurante(rta_obtenerRestaurante* obtenerRestaurante){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	obtenerRestaurante->cantCocineroAfinidad = obtenerRestaurante->cocineroAfinidad->elements_count;
	obtenerRestaurante->cantRecetas = obtenerRestaurante->recetas->elements_count;

	int size_recetas = tamanio_lista_strings(obtenerRestaurante->recetas);
	int size_cocineroAfinidad = tamanio_lista_cocineroAfinidad(obtenerRestaurante->recetas);

	buffer -> size = size_recetas + size_cocineroAfinidad + sizeof(uint32_t) * 3;

	void* stream = malloc(buffer -> size);
	int offset = 0;

	memcpy(stream + offset, &obtenerRestaurante->cantCocineroAfinidad, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_cocineroAfinidad* cocineroAfinidad;
	for(int i = 0; i < obtenerRestaurante->cantCocineroAfinidad; i++){
		cocineroAfinidad = list_get(obtenerRestaurante->cocineroAfinidad, i);
		memcpy(stream + offset, &cocineroAfinidad->idCocinero, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &cocineroAfinidad->afinidad.largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, cocineroAfinidad->afinidad.nombre, cocineroAfinidad->afinidad.largo_nombre);
		offset += cocineroAfinidad->afinidad.largo_nombre;
	}

	memcpy(stream + offset, &obtenerRestaurante->cantRecetas, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_nombre* nombre_receta;
	for(int i = 0; i < obtenerRestaurante->cantRecetas; i++){
		nombre_receta->nombre = list_get(obtenerRestaurante->recetas, i);
		nombre_receta->largo_nombre = strlen(nombre_receta->nombre);
		memcpy(stream + offset, &nombre_receta->largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, nombre_receta->nombre, nombre_receta->largo_nombre);
		offset += nombre_receta->largo_nombre;
	}

	memcpy(stream + offset, &obtenerRestaurante->cantHornos, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &obtenerRestaurante->posicion.x, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &obtenerRestaurante->posicion.y, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	return buffer;
}

rta_obtenerRestaurante* deserializacion_rta_obtener_restaurante(void* buffer){
	rta_obtenerRestaurante* obtenerRestaurante = malloc(sizeof(rta_obtenerRestaurante));

	obtenerRestaurante->cocineroAfinidad = list_create();
	obtenerRestaurante->recetas = list_create();

	memcpy(&obtenerRestaurante->cantCocineroAfinidad, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_cocineroAfinidad* cocineroAfinidad;

	for(int i = 0; i < obtenerRestaurante->cantCocineroAfinidad; i++){
		memcpy(&cocineroAfinidad->idCocinero, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		memcpy(&cocineroAfinidad->afinidad.largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		cocineroAfinidad->afinidad.nombre = malloc(cocineroAfinidad->afinidad.largo_nombre);
		memcpy(cocineroAfinidad->afinidad.nombre, buffer, cocineroAfinidad->afinidad.largo_nombre);
		buffer += cocineroAfinidad->afinidad.largo_nombre;
		list_add(obtenerRestaurante->cocineroAfinidad, cocineroAfinidad);
	}

	memcpy(&obtenerRestaurante->cantRecetas, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_nombre* nombre_receta;
	for(int i = 0; i < obtenerRestaurante->cantRecetas; i++){
		memcpy(&nombre_receta->largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		nombre_receta = malloc(nombre_receta->largo_nombre);
		memcpy(nombre_receta->nombre, buffer, nombre_receta->largo_nombre);
		buffer += nombre_receta->largo_nombre;
		list_add(obtenerRestaurante->recetas, nombre_receta);
	}

	memcpy(&obtenerRestaurante->cantHornos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&obtenerRestaurante->posicion.x, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&obtenerRestaurante->posicion.y, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);


	return obtenerRestaurante;

}

int tamanio_lista_strings(t_list* lista_de_strings){
	int tamanio = 0;

	char* string;

	for(int i = 0; i < lista_de_strings->elements_count; i++){
		string = list_get(lista_de_strings, i);
		tamanio += strlen(string) + sizeof(uint32_t);
	}

	return tamanio;
}

int tamanio_lista_cocineroAfinidad(t_list* lista_de_strings){
	int tamanio = 0;

	t_cocineroAfinidad* cocineroAfinidad;

	for(int i = 0; i < lista_de_strings->elements_count; i++){
		cocineroAfinidad = list_get(lista_de_strings, i);
		cocineroAfinidad->afinidad.largo_nombre = strlen(cocineroAfinidad->afinidad.nombre);
		tamanio += cocineroAfinidad->afinidad.largo_nombre + sizeof(uint32_t)*2;
	}

	return tamanio;
}


