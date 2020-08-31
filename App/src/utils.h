#ifndef UTILS_H_
#define UTILS_H_

#include <stdlib.h>
#include <commons/config.h>
#include <commons/log.h>
#include <commons/collections/list.h>
#include <commons/string.h>

typedef enum {
    FIFO,
    HRRN,
    SJF_SD
} t_algoritmoPlanificacion;

typedef struct {
    int x;
    int y;
    int frecuenciaDescanso;
    int tiempoDescanso;
} t_repartidor;

// variables de logging
    t_log*  logger_obligatorio;
    bool    logger_obligatorio_consolaActiva;

    t_log*  logger_configuracion;
    bool    logger_configuracion_consolaActiva;
    char*   logger_configuracion_path;

char* cfval_ipComanda;
int cfval_puertoComanda;
int cfval_puertoEscucha;
int cfval_retardoCicloCPU;
int cfval_gradoMultiprocesamiento;
t_algoritmoPlanificacion cfval_algoritmoPlanificacion;
double cfval_alpha;
double cfval_estimacionInicial;
char** cfval_platosDefault;
int cfval_posicionRestDefaultX;
int cfval_posicionRestDefaultY;

t_list* repartidores;

void configuracionInicial(void);

#endif // UTILS_H_