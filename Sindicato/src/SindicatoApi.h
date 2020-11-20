#ifndef SINDICATOAPI_H_
#define SINDICATOAPI_H_

#include "SindicatoUtils.h"

#define D_CANTIDAD_COCINEROS "CANTIDAD_COCINEROS"
#define D_POSICION "POSICION"
#define D_AFINIDAD_COCINEROS "AFINIDAD_COCINEROS"
#define D_PLATOS "PLATOS"
#define D_PRECIO_PLATOS "PRECIO_PLATOS"
#define D_CANTIDAD_HORNOS "CANTIDAD_HORNOS"
#define D_CANTIDAD_PEDIDOS "CANTIDAD_PEDIDOS"


#define D_PASOS "PASOS"
#define D_TIEMPO_PASOS "TIEMPO_PASOS"

#define D_ESTADO_PEDIDO "ESTADO_PEDIDO"
#define D_LISTA_PLATOS "LISTA_PLATOS"
#define D_CANTIDAD_PLATOS "CANTIDAD_PLATOS"
#define D_CANTIDAD_LISTA "CANTIDAD_LISTA"
#define D_PRECIO_TOTAL "PRECIO_TOTAL"

#define E_CONFIRMADO "CONFIRMADO"
#define E_PENDIENTE "PENDIENTE"
#define E_TERMINADO "TERMINADO"

typedef enum{
	TYPE_RECETA = 0,
	TYPE_PEDIDO = 1,
	TYPE_RESTAURANTE = 2,
}file_type;

typedef enum{
	MODE_ADD = 0,
	MODE_UPDATE = 1,
	MODE_READ = 2,
}mode_fs;

typedef struct{
	int cantidad_cocineros;
	t_coordenadas posicion;
	t_list* afinidad_cocineros; //lista de t_nombre (afinidades)
	t_list* platos; //lista de t_nombre (platos)
	t_list* precios; //lista de precios
	int cantidad_hornos;
	int cantidad_pedidos;
}t_restaurante_file;

typedef struct{
	t_list* pasos; //lista de t_paso
}t_receta_file;

typedef struct{
	est_pedido estado_pedido;
	t_list* platos; //lista de t_nombre (platos)
	t_list* cantidad_platos; //lista de int
	t_list* cantidad_lista; //lista de int
	int precio_total;
}t_pedido_file;

typedef struct{
	uint32_t initialBlock;
	int stringSize;
}t_initialBlockInfo;


/* ********************************** PUBLIC  FUNCTIONS ********************************** */

/* Console functions */
void sindicato_api_crear_restaurante(char* nombre, char* cantCocineros, char* posXY, char* afinidadCocinero, char* platos, char* precioPlatos, char* cantHornos);
void sindicato_api_crear_receta(char* nombre, char* pasos, char* tiempoPasos);

/* Server functions */
void sindicato_api_send_response_of_operation(t_responseMessage* response);
t_restaurante_y_plato* sindicato_api_consultar_platos(void* message);
uint32_t* sindicato_api_guardar_pedido(void* pedido);
uint32_t* sindicato_api_guardar_plato(void* pedido);
uint32_t* sindicato_api_confirmar_pedido(void* pedido);
rta_obtenerPedido* sindicato_api_obtener_pedido(void* pedido);
rta_obtenerRestaurante* sindicato_api_obtener_restaurante(void* restaurante);
uint32_t* sindicato_api_plato_listo(void* plato);
rta_obtenerReceta* sindicato_api_obtener_receta(void* plato);
uint32_t* sindicato_api_terminar_pedido(void* pedido);

/* Main functions */
void sindicato_api_afip_initialize();

#endif /* SINDICATOAPI_H_ */
