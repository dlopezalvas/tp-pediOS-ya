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
		case(RTA_CONSULTAR_RESTAURANTES): return buffer_rta_consultar_restaurantes(parametros); //TODO lista
		case(SELECCIONAR_RESTAURANTE): return buffer_seleccionar_restaurante(parametros);
		case(RTA_SELECCIONAR_RESTAURANTE): return buffer_id_o_confirmacion(parametros);
		case(OBTENER_RESTAURANTE): return buffer_nombre_restaurante(parametros);
		case(RTA_OBTENER_RESTAURANTE): return buffer_rta_obtener_restaurantes(parametros); //TODO lista
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
	memcpy(seleccionarRestaurante->restaurante.nombre, buffer, seleccionarRestaurante->restaurante.largo_nombre);
	seleccionarRestaurante->restaurante.nombre[seleccionarRestaurante->restaurante.largo_nombre] = '\0';

	return seleccionarRestaurante;
}

//confirmacion y id pedido

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

//nombre restaurante


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
	memcpy(nombre_restaurante->nombre, buffer, nombre_restaurante->largo_nombre);
	nombre_restaurante->nombre[nombre_restaurante->largo_nombre] = '\0';

	return nombre_restaurante;
}


//id y nombre

t_buffer* buffer_nombre_id_pedido(t_nombre_y_id* nombre_y_id){
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

t_nombre_y_id* deserializar_nombre_id_pedido(void* buffer){

	t_nombre_y_id* nombre_y_id = malloc(sizeof(m_seleccionarRestaurante));
	memcpy(&nombre_y_id->id, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&nombre_y_id->nombre.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
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
	memcpy(guardar_plato->comida.nombre, buffer, guardar_plato->comida.largo_nombre);
	buffer += guardar_plato->comida.largo_nombre;
	guardar_plato->comida.nombre[guardar_plato->comida.largo_nombre] = '\0';
	memcpy(&guardar_plato->cantidad, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&guardar_plato->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
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
	memcpy(plato_listo->comida.nombre, buffer, plato_listo->comida.largo_nombre);
	buffer += plato_listo->comida.largo_nombre;
	plato_listo->comida.nombre[plato_listo->comida.largo_nombre] = '\0';
	memcpy(&plato_listo->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(plato_listo->restaurante.nombre, buffer, plato_listo->restaurante.largo_nombre);
	plato_listo->restaurante.nombre[plato_listo->restaurante.largo_nombre] = '\0';

	return plato_listo;
}

//buffer vacio

t_buffer* buffer_vacio(){
	t_buffer* buffer = malloc(sizeof(t_buffer));
	buffer->size = 0;
	return buffer;
}

//rta consultar restaurantes
//t_buffer* buffer_rta_consultar_restaurantes(rta_consultarRestaurantes* lista_restaurantes){
//	t_buffer* buffer = malloc(sizeof(t_buffer));
//
//	lista_restaurantes->cantRestaurantes = lista_restaurantes->restaurantes->elements_count;
//
//	int size_strings = tamanio_lista_strings(lista_restaurantes->restaurantes);
//
//	buffer -> size = sizeof(uint32_t) + size_strings;
//
//	void* stream = malloc(buffer -> size);
//	int offset = 0;
//
//	memcpy(stream + offset, &lista_restaurantes->cantRestaurantes, sizeof(uint32_t));
//	offset += sizeof(uint32_t);
//
//	t_nombre* nombre_restaurante;
//
//	for(int i = 0; i < lista_restaurantes->cantRestaurantes; i++){
//
//		nombre_restaurante->largo_nombre = strlen(list_get(lista_restaurantes, i));
//		memcpy(stream + offset, &pos_x, sizeof(uint32_t));
//		offset += sizeof(uint32_t);
//		memcpy(stream + offset, &pos_y, sizeof(uint32_t));
//		offset += sizeof(uint32_t);
//	}
//
//	memcpy(stream + offset, &localized_pokemon.nombre.largo_nombre, sizeof(uint32_t));
//	offset += sizeof(uint32_t);
//	memcpy(stream + offset, localized_pokemon.nombre.nombre, localized_pokemon.nombre.largo_nombre);
//
//
//	buffer -> stream = stream;
//	return buffer;
//}

