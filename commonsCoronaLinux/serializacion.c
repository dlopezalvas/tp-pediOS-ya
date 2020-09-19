#include "serializacion.h"

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

void* deserializar_mensaje(void* buffer, op_code tipo_mensaje){

	struct_code tipo_struct = op_code_to_struct_code(tipo_mensaje);

	switch(tipo_struct){
	case STRC_RESTAURANTE_Y_PLATO:return deserializar_restaurante_y_plato(buffer);

	case STRC_SELECCIONAR_RESTAURANTE:return deserializar_seleccionar_restaurante(buffer);

	case STRC_ID_CONFIRMACION: return deserializar_id_o_confirmacion(buffer);

	case STRC_NOMBRE: return deserializar_nombre(buffer);

	case STRC_RTA_OBTENER_RESTAURANTE:return deserializar_rta_obtener_restaurante(buffer);

	case STRC_NOMBRE_ID: return deserializar_nombre_y_id(buffer);

	case STRC_PLATO_LISTO:return deserializar_plato_listo(buffer);

	case STRC_RTA_CONSULTAR_PEDIDO:return deserializar_rta_consultar_pedido(buffer);

	case STRC_RTA_OBTENER_PEDIDO:return deserializar_rta_obtener_pedido(buffer);

	case STRC_GUARDAR_PLATO:return deserializar_guardar_plato(buffer);

	case STRC_POSICION:return deserializar_posicion(buffer);

	case STRC_RTA_OBTENER_RECETA: return deserializar_rta_obtener_receta(buffer);
	}
	return NULL;
}

t_buffer* cargar_buffer(t_mensaje* mensaje){
	void* parametros = mensaje -> parametros;

	struct_code tipo_struct = op_code_to_struct_code(mensaje -> tipo_mensaje);

	switch(tipo_struct){
	case STRC_MENSAJE_VACIO: return buffer_vacio();

	case STRC_RESTAURANTE_Y_PLATO:return buffer_restaurante_y_plato(parametros);

	case STRC_SELECCIONAR_RESTAURANTE:return buffer_seleccionar_restaurante(parametros);

	case STRC_ID_CONFIRMACION: return buffer_id_o_confirmacion(parametros);

	case STRC_NOMBRE: return buffer_nombre(parametros);

	case STRC_RTA_OBTENER_RESTAURANTE:return buffer_rta_obtener_restaurante(parametros);

	case STRC_NOMBRE_ID: return buffer_nombre_y_id(parametros);

	case STRC_PLATO_LISTO:return buffer_plato_listo(parametros);

	case STRC_RTA_CONSULTAR_PEDIDO:return buffer_rta_consultar_pedido(parametros);

	case STRC_RTA_OBTENER_PEDIDO:return buffer_rta_obtener_pedido(parametros);

	case STRC_GUARDAR_PLATO:return buffer_guardar_plato(parametros);

	case STRC_POSICION:return buffer_posicion(parametros);

	case STRC_RTA_OBTENER_RECETA: return buffer_rta_obtener_receta(parametros);
	}
	return NULL;

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

	memcpy(stream, numero, sizeof(uint32_t));

	buffer -> stream = stream;

	return buffer;
}

uint32_t* deserializar_id_o_confirmacion(void* buffer){

	uint32_t* numero = malloc(sizeof(uint32_t));
	memcpy(numero, buffer, sizeof(uint32_t));

	return numero;
}

//nombre restaurante: OBTENER_RESTAURANTE - CONSULTAR_PLATOS - OBTENER_RECETA


t_buffer* buffer_nombre(t_nombre* nombre){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	nombre->largo_nombre = strlen(nombre->nombre);

	buffer -> size = sizeof(uint32_t) + nombre->largo_nombre;
	int offset = 0;
	void* stream = malloc(buffer -> size);

	memcpy(stream + offset, &nombre->largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, nombre->nombre, nombre->largo_nombre);

	buffer -> stream = stream;

	return buffer;
}

t_nombre* deserializar_nombre(void* buffer){
	t_nombre* nombre = malloc(sizeof(t_nombre));

	memcpy(&nombre->largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	nombre->nombre = malloc(nombre->largo_nombre + 1);
	memcpy(nombre->nombre, buffer, nombre->largo_nombre);
	nombre->nombre[nombre->largo_nombre] = '\0';

	return nombre;
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

//rta consultar restaurantes - rta consultar_platos
t_buffer* buffer_restaurante_y_plato(t_restaurante_y_plato* lista_nombres){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	lista_nombres->cantElementos = lista_nombres->nombres->elements_count;

	int size_strings = tamanio_lista_strings(lista_nombres->nombres);

	buffer -> size = sizeof(uint32_t) + size_strings;

	void* stream = malloc(buffer -> size);
	int offset = 0;

	memcpy(stream + offset, &lista_nombres->cantElementos, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_nombre* nombre;

	for(int i = 0; i < lista_nombres->cantElementos; i++){
		nombre->nombre = list_get(lista_nombres->nombres, i);
		nombre->largo_nombre = strlen(nombre->nombre);
		memcpy(stream + offset, &nombre->largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, nombre->nombre, nombre->largo_nombre);
		offset += nombre->largo_nombre;
	}

	buffer -> stream = stream;
	return buffer;
}

t_restaurante_y_plato* deserializar_restaurante_y_plato(void* buffer){

	t_restaurante_y_plato* lista_nombres = malloc(sizeof(t_restaurante_y_plato));

	lista_nombres->nombres = list_create();

	memcpy(&lista_nombres->cantElementos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_nombre* nombre;

	for(int i = 0; i < lista_nombres->cantElementos; i++){
		memcpy(&nombre->largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		nombre->nombre = malloc(nombre->largo_nombre +1);
		memcpy(nombre->nombre, buffer, nombre->largo_nombre);
		nombre->nombre[nombre->largo_nombre] = '\0';
		buffer += nombre->largo_nombre;
		list_add(lista_nombres->nombres, nombre->nombre);
	}

	return lista_nombres;
}

//rta obtener restaurante

t_buffer* buffer_rta_obtener_restaurante(rta_obtenerRestaurante* obtenerRestaurante){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	obtenerRestaurante->cantCocineroAfinidad = obtenerRestaurante->cocineroAfinidad->elements_count;
	obtenerRestaurante->cantRecetas = obtenerRestaurante->recetas->elements_count;

	int size_recetas = tamanio_lista_recetas(obtenerRestaurante->recetas);
	int size_cocineroAfinidad = tamanio_lista_cocineroAfinidad(obtenerRestaurante->recetas);

	buffer -> size = size_recetas + size_cocineroAfinidad + sizeof(uint32_t) * 5;

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

	t_receta* receta;
	for(int i = 0; i < obtenerRestaurante->cantRecetas; i++){
		receta = list_get(obtenerRestaurante->recetas, i);
		memcpy(stream + offset, &receta->precio, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		receta->receta.largo_nombre = strlen(receta->receta.nombre);
		memcpy(stream + offset, &receta->receta.largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, receta->receta.nombre, receta->receta.largo_nombre);
		offset += receta->receta.largo_nombre;
	}

	memcpy(stream + offset, &obtenerRestaurante->cantHornos, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &obtenerRestaurante->posicion.x, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &obtenerRestaurante->posicion.y, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	buffer->stream = stream;

	return buffer;
}

rta_obtenerRestaurante* deserializar_rta_obtener_restaurante(void* buffer){
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
		cocineroAfinidad->afinidad.nombre = malloc(cocineroAfinidad->afinidad.largo_nombre +1);
		memcpy(cocineroAfinidad->afinidad.nombre, buffer, cocineroAfinidad->afinidad.largo_nombre);
		cocineroAfinidad->afinidad.nombre[cocineroAfinidad->afinidad.largo_nombre] = '\0';
		buffer += cocineroAfinidad->afinidad.largo_nombre;
		list_add(obtenerRestaurante->cocineroAfinidad, cocineroAfinidad);
	}

	memcpy(&obtenerRestaurante->cantRecetas, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_receta* receta;
	for(int i = 0; i < obtenerRestaurante->cantRecetas; i++){
		memcpy(&receta->precio, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		memcpy(&receta->receta.largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		receta->receta.nombre = malloc(receta->receta.largo_nombre +1);
		memcpy(receta->receta.nombre, buffer, receta->receta.largo_nombre);
		receta->receta.nombre[receta->receta.largo_nombre] = '\0';
		buffer += receta->receta.largo_nombre;
		list_add(obtenerRestaurante->recetas, receta);
	}

	memcpy(&obtenerRestaurante->cantHornos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&obtenerRestaurante->posicion.x, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&obtenerRestaurante->posicion.y, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);


	return obtenerRestaurante;

}

// rta_consultar_pedido

t_buffer* buffer_rta_consultar_pedido(rta_consultarPedido* consultarPedido){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	consultarPedido->cantPlatos = consultarPedido->platos->elements_count;

	int size_lista_platos = tamanio_lista_platos_con_estado(consultarPedido->platos);

	buffer->size = size_lista_platos + consultarPedido->restaurante.largo_nombre + sizeof(est_pedido) + sizeof(uint32_t)*3;

	void* stream = malloc(buffer -> size);
	int offset = 0;

	memcpy(stream + offset, &consultarPedido->cantPlatos, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_plato_con_estado* plato_con_estado;
	for(int i = 0; i < consultarPedido->cantPlatos; i++){
		plato_con_estado = list_get(consultarPedido->platos, i);
		memcpy(stream + offset, &plato_con_estado->estadoPlato, sizeof(est_plato));
		offset += sizeof(est_plato);
		memcpy(stream + offset, &plato_con_estado->plato.largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, plato_con_estado->plato.nombre, plato_con_estado->plato.largo_nombre);
		offset += plato_con_estado->plato.largo_nombre;
	}

	memcpy(stream + offset, &consultarPedido->idRepartidor, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &consultarPedido->estadoPedido, sizeof(est_pedido));
	offset += sizeof(est_pedido);
	memcpy(stream + offset, &consultarPedido->restaurante.largo_nombre, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, consultarPedido->restaurante.nombre, consultarPedido->restaurante.largo_nombre);
	offset += consultarPedido->restaurante.largo_nombre;

	return buffer;
}

rta_consultarPedido* deserializar_rta_consultar_pedido(void* buffer){
	rta_consultarPedido* consultarPedido = malloc(sizeof(rta_consultarPedido));

	consultarPedido->platos = list_create();

	memcpy(&consultarPedido->cantPlatos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_plato_con_estado* plato_con_estado;

	for(int i = 0; i < consultarPedido->cantPlatos; i++){
		memcpy(&plato_con_estado->estadoPlato, buffer, sizeof(est_plato));
		buffer += sizeof(consultarPedido);
		memcpy(&plato_con_estado->plato.largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		plato_con_estado->plato.nombre = malloc(plato_con_estado->plato.largo_nombre +1);
		memcpy(plato_con_estado->plato.nombre, buffer, plato_con_estado->plato.largo_nombre);
		plato_con_estado->plato.nombre[plato_con_estado->plato.largo_nombre] = '\0';
		buffer += plato_con_estado->plato.largo_nombre;
		list_add(consultarPedido->platos, plato_con_estado);
	}

	memcpy(&consultarPedido->idRepartidor, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&consultarPedido->estadoPedido, buffer, sizeof(est_pedido));
	buffer += sizeof(est_pedido);
	memcpy(&consultarPedido->restaurante.largo_nombre, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	consultarPedido->restaurante.nombre = malloc(consultarPedido->restaurante.largo_nombre);
	memcpy(consultarPedido->restaurante.nombre, buffer, consultarPedido->restaurante.largo_nombre);
	consultarPedido->restaurante.nombre[consultarPedido->restaurante.largo_nombre] = '\0';

	return consultarPedido;
}

//rta obtener pedido

t_buffer* buffer_rta_obtener_pedido(rta_obtenerPedido* obtenerPedido){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	obtenerPedido->cantPedidos = obtenerPedido->infoPedidos->elements_count;

	int size_pedidos = tamanio_lista_pedidos(obtenerPedido->infoPedidos);

	buffer->size = size_pedidos + sizeof(uint32_t);

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &obtenerPedido->cantPedidos, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_elemPedido* elemPedido;
	for(int i = 0; i < obtenerPedido->cantPedidos; i++){
		elemPedido = list_get(obtenerPedido->infoPedidos, i);
		memcpy(stream + offset, &elemPedido->cantHecha, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, &elemPedido->cantTotal, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		elemPedido->comida.largo_nombre = strlen(elemPedido->comida.nombre);
		memcpy(stream + offset, &elemPedido->comida.largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, elemPedido->comida.nombre, elemPedido->comida.largo_nombre);
		offset += elemPedido->comida.largo_nombre;
	}

	buffer->stream = stream;

	return buffer;
}

rta_obtenerPedido* deserializar_rta_obtener_pedido(void* buffer){
	rta_obtenerPedido* obtenerPedido = malloc(sizeof(rta_obtenerPedido));

	obtenerPedido->infoPedidos = list_create();

	memcpy(&obtenerPedido->cantPedidos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_elemPedido* elemPedido;

	for(int i = 0; i < obtenerPedido->cantPedidos; i++){
		memcpy(&elemPedido->cantHecha, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		memcpy(&elemPedido->cantTotal, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		memcpy(&elemPedido->comida.largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		elemPedido->comida.nombre = malloc(elemPedido->comida.largo_nombre + 1);
		memcpy(elemPedido->comida.nombre, buffer, elemPedido->comida.largo_nombre);
		elemPedido->comida.nombre[elemPedido->comida.largo_nombre] = '\0';
		buffer += elemPedido->comida.largo_nombre;
		list_add(obtenerPedido->infoPedidos, elemPedido);
	}

	return obtenerPedido;
}

//posicion cliente


t_buffer* buffer_posicion(t_coordenadas* posicion){
	t_buffer* buffer = malloc(sizeof(t_buffer));


	buffer -> size = sizeof(uint32_t)*2;
	void* stream = malloc(buffer -> size);
	uint32_t offset = 0;
	memcpy(stream + offset, &posicion->x, sizeof(uint32_t));
	offset += sizeof(uint32_t);
	memcpy(stream + offset, &posicion->y, sizeof(uint32_t));
	buffer -> stream = stream;

	return buffer;
}

t_coordenadas* deserializar_posicion(void* buffer){

	t_coordenadas* posicion = malloc(sizeof(t_coordenadas));
	memcpy(&posicion->x, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);
	memcpy(&posicion->y, buffer, sizeof(uint32_t));

	return posicion;
}

//rta_obtenerReceta
t_buffer* buffer_rta_obtener_receta(rta_obtenerReceta* obtenerReceta){
	t_buffer* buffer = malloc(sizeof(t_buffer));

	obtenerReceta->cantPasos = obtenerReceta->pasos->elements_count;

	int size_pedidos = tamanio_lista_pasos(obtenerReceta->pasos);

	buffer->size = size_pedidos + sizeof(uint32_t);

	void* stream = malloc(buffer->size);
	int offset = 0;

	memcpy(stream + offset, &obtenerReceta->cantPasos, sizeof(uint32_t));
	offset += sizeof(uint32_t);

	t_paso* paso;
	for(int i = 0; i < obtenerReceta->cantPasos; i++){
		paso = list_get(obtenerReceta->pasos, i);
		memcpy(stream + offset, &paso->duracion, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		paso->paso.largo_nombre = strlen(paso->paso.nombre);
		memcpy(stream + offset, &paso->paso.largo_nombre, sizeof(uint32_t));
		offset += sizeof(uint32_t);
		memcpy(stream + offset, paso->paso.nombre, paso->paso.largo_nombre);
		offset += paso->paso.largo_nombre;
	}

	buffer->stream = stream;

	return buffer;
}

rta_obtenerReceta* deserializar_rta_obtener_receta(void* buffer){
	rta_obtenerReceta* obtenerReceta = malloc(sizeof(rta_obtenerReceta));

	obtenerReceta->pasos = list_create();

	memcpy(&obtenerReceta->cantPasos, buffer, sizeof(uint32_t));
	buffer += sizeof(uint32_t);

	t_paso* paso;

	for(int i = 0; i < obtenerReceta->cantPasos; i++){
		memcpy(&paso->duracion, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		memcpy(&paso->paso.largo_nombre, buffer, sizeof(uint32_t));
		buffer += sizeof(uint32_t);
		paso->paso.nombre = malloc(paso->paso.largo_nombre + 1);
		memcpy(paso->paso.nombre, buffer, paso->paso.largo_nombre);
		paso->paso.nombre[paso->paso.largo_nombre] = '\0';
		buffer += paso->paso.largo_nombre;
		list_add(obtenerReceta->pasos, paso);
	}

	return obtenerReceta;
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

int tamanio_lista_cocineroAfinidad(t_list* lista_cocinerosAfinidades){
	int tamanio = 0;

	t_cocineroAfinidad* cocineroAfinidad;

	for(int i = 0; i < lista_cocinerosAfinidades->elements_count; i++){
		cocineroAfinidad = list_get(lista_cocinerosAfinidades, i);
		cocineroAfinidad->afinidad.largo_nombre = strlen(cocineroAfinidad->afinidad.nombre);
		tamanio += cocineroAfinidad->afinidad.largo_nombre + sizeof(uint32_t)*2;
	}

	return tamanio;
}

int tamanio_lista_platos_con_estado(t_list* lista_de_platos){
	int tamanio = 0;

	t_plato_con_estado* plato;

	for(int i = 0; i < lista_de_platos->elements_count; i++){
		plato = list_get(lista_de_platos, i);
		plato->plato.largo_nombre = strlen(plato->plato.nombre);
		tamanio += plato->plato.largo_nombre + sizeof(uint32_t) + sizeof(est_plato);
	}

	return tamanio;
}

int tamanio_lista_pasos(t_list* lista_de_pasos){
	int tamanio = 0;

	t_paso* paso;

	for(int i = 0; i < lista_de_pasos->elements_count; i++){
		paso = list_get(lista_de_pasos, i);
		paso->paso.largo_nombre = strlen(paso->paso.nombre);
		tamanio += paso->paso.largo_nombre + sizeof(uint32_t)*2;
	}

	return tamanio;
}

int tamanio_lista_recetas(t_list* lista_recetas){ //TODO unificar t_paso con t_receta
	int tamanio = 0;

	t_receta* receta;

	for(int i = 0; i < lista_recetas->elements_count; i++){
		receta = list_get(lista_recetas, i);
		receta->receta.largo_nombre = strlen(receta->receta.nombre);
		tamanio += receta->receta.largo_nombre + sizeof(uint32_t)*2;
	}

	return tamanio;
}

int tamanio_lista_pedidos(t_list* lista_pedidos){
	int tamanio = 0;

	t_elemPedido* elemPedido;

	for(int i = 0; i < lista_pedidos->elements_count; i++){
		elemPedido = list_get(lista_pedidos, i);
		elemPedido->comida.largo_nombre = strlen(elemPedido->comida.nombre);
		tamanio += elemPedido->comida.largo_nombre + sizeof(uint32_t) * 3;
	}

	return tamanio;
}

