#ifndef APP_H_
#define APP_H_

#include "utilsApp.h"

void mock_mensajes(void);
t_restaurante* mock_registrar_restaurante(char* nombre, int x, int y);
void mock_registrar_cliente(
		unsigned id,
		int x,
		int y,
		int pedido_id,
		t_restaurante* restaurante_seleccionado
);
void mock_confirmar_pedido(int id);

#endif // APP_H_