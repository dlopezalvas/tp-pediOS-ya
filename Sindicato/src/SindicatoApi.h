#ifndef SINDICATOAPI_H_
#define SINDICATOAPI_H_

#include <stdio.h>
#include <stdlib.h>

#include "../commonsCoronaLinux/logs.h"
#include "../commonsCoronaLinux/socket.h"
#include "../commonsCoronaLinux/utils.h"

void sindicato_api_crear_restaurante(char* nombre, int cantCocineros, int* posXY, char** afinidades, char** platos, int* precioPlatos, int cantHornos);
void sindicato_api_crear_receta(char* nombre, char** pasos, int* tiempoPasos);

#endif /* SINDICATOAPI_H_ */
