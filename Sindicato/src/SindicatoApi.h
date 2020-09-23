#ifndef SINDICATOAPI_H_
#define SINDICATOAPI_H_

#include "SindicatoUtils.h"

/* ********************************** PUBLIC  FUNCTIONS ********************************** */

void sindicato_api_crear_restaurante(char* nombre, int cantCocineros, int* posXY, char** afinidades, char** platos, int* precioPlatos, int cantHornos);
void sindicato_api_crear_receta(char* nombre, char** pasos, int* tiempoPasos);
void sindicato_afip_initialize();

#endif /* SINDICATOAPI_H_ */
