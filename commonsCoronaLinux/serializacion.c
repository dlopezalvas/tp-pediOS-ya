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
		case(CONSULTAR_RESTAURANTES): return buffer_consultar_restaurantes(parametros); //no tiene nada TODO
		case(RTA_CONSULTAR_RESTAURANTES): return buffer_rta_consultar_restaurantes(parametros); //TODO lista
		case(SELECCIONAR_RESTAURANTE): return buffer_seleccionar_restaurantes(parametros);
		case(RTA_SELECCIONAR_RESTAURANTE): return buffer_confirmacion(parametros);
		case(OBTENER_RESTAURANTE): return buffer_nombre_restaurante(parametros);
		case(RTA_OBTENER_RESTAURANTE): return buffer_rta_obtener_restaurantes(parametros); //TODO lista
		case(CONSULTAR_PLATOS): return buffer_nombre_restaurante(parametros);
		case(RTA_CONSULTAR_PLATOS): return buffer_rta_consultar_platos(parametros); //TODO lista
		case(CREAR_PEDIDO): return buffer_crear_pedido(parametros); //no tiene nada TODO
		case(RTA_CREAR_PEDIDO): return buffer_id_pedido(parametros);
		case(GUARDAR_PEDIDO): return buffer_restaurante_id_pedido(parametros);
		case(RTA_GUARDAR_PEDIDO): return buffer_id_pedido(parametros);
		case(AGREGAR_PLATO): return buffer_agregar_plato(parametros);
		case(RTA_AGREGAR_PLATO): return buffer_confirmacion(parametros);
		case(GUARDAR_PLATO): return buffer_guardar_plato(parametros);
		case(RTA_GUARDAR_PLATO): return buffer_confirmacion(parametros);
		case(CONFIRMAR_PEDIDO): return buffer_id_pedido(parametros);
		case(RTA_CONFIRMAR_PEDIDO): return buffer_confirmacion(parametros);
		case(PLATO_LISTO): return buffer_plato_listo(parametros);
		case(RTA_PLATO_LISTO): return buffer_confirmacion(parametros);
		case(CONSULTAR_PEDIDO): return buffer_id_pedido(parametros);
		case(RTA_CONSULTAR_PEDIDO): return buffer_rta_consultar_pedido(parametros); //TODO lista
		case(OBTENER_PEDIDO): return buffer_restaurante_id_pedido(parametros);
		case(RTA_OBTENER_PEDIDO): return buffer_rta_obtener_pedido(parametros); //TODO lista
		case(FINALIZAR_PEDIDO): return buffer_restaurante_id_pedido(parametros);
		case(RTA_FINALIZAR_PEDIDO): return buffer_confirmacion(parametros);
		case(TERMINAR_PEDIDO): return buffer_restaurante_id_pedido(parametros);
		case(RTA_TERMINAR_PEDIDO): return buffer_confirmacion(parametros);
	}
	return 0;
}

///////////////////////////SERIALIZAR/////////////////////////////////


t_buffer* buffer_seleccionar_restaurantes(m_seleccionarRestaurante* seleccionarRestaurante){
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


