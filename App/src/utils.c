#include "utils.h"

// configuracionInicial() inicializa:
//      > los loggers
//      > todos los cfval_* (config values)
//      > la lista de repartidores
void configuracionInicial(void) {
    // auxiliares
    char** posicionesRepartidores;
    char** frecuenciasDescanso;
    char** tiemposDescanso;
    char* pathArchivoLog;
    char* algoritmoPlanificacion;
    t_repartidor* repartidor;

    // para los loggers
    char* program_name = "App";

    // config local, todos los valores finales se guardan en variables globales
    t_config* config = config_create("App.config");

    // configuracion de loggers adicionales para debug
    logger_configuracion = log_create(logger_configuracion_path, program_name, logger_configuracion_consolaActiva, LOG_LEVEL_DEBUG);

    // configuracion del logger para el log obligatorio
    pathArchivoLog = config_get_string_value(config, "ARCHIVO_LOG");
        log_debug(logger_configuracion, "[CONFIG] Path del log obligatorio: %s", pathArchivoLog);
    logger_obligatorio = log_create(pathArchivoLog, program_name, logger_obligatorio_consolaActiva, LOG_LEVEL_INFO);

    // configuracion de ip y puertos
    cfval_ipComanda = config_get_string_value(config, "IP_COMANDA");
        log_debug(logger_configuracion, "[CONFIG] IP de Comanda: %s", cfval_ipComanda);
    cfval_puertoComanda = config_get_int_value(config, "PUERTO_COMANDA");
        log_debug(logger_configuracion, "[CONFIG] Puerto de Comanda: %i", cfval_puertoComanda);
    cfval_puertoEscucha = config_get_int_value(config, "PUERTO_ESCUCHA");
        log_debug(logger_configuracion, "[CONFIG] Puerto de escucha: %i", cfval_puertoEscucha);

    // configuracion del procesador
    cfval_retardoCicloCPU = config_get_int_value(config, "RETARDO_CICLO_CPU");
        log_debug(logger_configuracion, "[CONFIG] Retardo de ciclo de CPU: %i", cfval_retardoCicloCPU);
    cfval_gradoMultiprocesamiento = config_get_int_value(config, "GRADO_DE_MULTIPROCESAMIENTO");
        log_debug(logger_configuracion, "[CONFIG] Grado de multiprocesamiento: %i", cfval_gradoMultiprocesamiento);

    // se settea en t_algoritmoPlanificacion
    algoritmoPlanificacion = config_get_string_value(config, "ALGORITMO_DE_PLANIFICACION");
        log_debug(logger_configuracion, "[CONFIG] Algoritmo de planificacion: %s", algoritmoPlanificacion);
    if (string_equals_ignore_case("FIFO", algoritmoPlanificacion))
        cfval_algoritmoPlanificacion = FIFO;
    if (string_equals_ignore_case("HRRN", algoritmoPlanificacion))
        cfval_algoritmoPlanificacion = HRRN;
    // se lee alpha y estimacion inicial solo si es SJF
    if (string_equals_ignore_case("SJF-SD", algoritmoPlanificacion)) {
        cfval_algoritmoPlanificacion = SJF_SD;
        cfval_alpha = config_get_double_value(config, "ALPHA");
            log_debug(logger_configuracion, "[CONFIG] Alpha: %i", cfval_alpha);
        cfval_estimacionInicial = config_get_double_value(config, "ESTIMACION_INICIAL");
            log_debug(logger_configuracion, "[CONFIG] Estimacion inicial: %i", cfval_estimacionInicial);
    }

    // configuracion de repartidores
    log_debug(logger_configuracion, "[CONFIG] Repartidores:");
    posicionesRepartidores = string_split(config_get_string_value(config, "REPARTIDORES"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
    frecuenciasDescanso = string_split(config_get_string_value(config, "FRECUENCIA_DE_DESCANSO"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
    tiemposDescanso = string_split(config_get_string_value(config, "TIEMPO_DE_DESCANSO"), "|"); //TODO: ojo separators para cuando corrijan el enunciado

    for (
        unsigned index_repartidores = 0;
        posicionesRepartidores[index_repartidores];
        index_repartidores++
    ) {
        log_debug(logger_configuracion, "[CONFIG] |\t<%i>", index_repartidores);
        repartidor = malloc(sizeof(t_repartidor));
        
        log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. X: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0])); //TODO: ojo separators para cuando corrijan el enunciado
        repartidor->x = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[0]); //TODO: ojo separators para cuando corrijan el enunciado
        
        log_debug(logger_configuracion, "[CONFIG] |\t|\tPos. Y: %i", atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1])); //TODO: ojo separators para cuando corrijan el enunciado
        repartidor->y = atoi(string_split(posicionesRepartidores[index_repartidores], ",")[1]); //TODO: ojo separators para cuando corrijan el enunciado
        
        log_debug(logger_configuracion, "[CONFIG] |\t|\tTiempo de descanso: %i", atoi(tiemposDescanso[index_repartidores]));
        repartidor->tiempoDescanso = atoi(tiemposDescanso[index_repartidores]);
        
        log_debug(logger_configuracion, "[CONFIG] |\t|\tFrecuencia de descanso: %i", atoi(frecuenciasDescanso[index_repartidores]));
        repartidor->frecuenciaDescanso = atoi(frecuenciasDescanso[index_repartidores]);
    }

    // configuracion del restaurant default
    cfval_platosDefault = string_split(config_get_string_value(config, "PLATOS_DEFAULT"), "|"); //TODO: ojo separators para cuando corrijan el enunciado
    log_debug(logger_configuracion, "[CONFIG] Platos del restaurant default:");
    for (
        unsigned index_platos = 0;
        cfval_platosDefault[index_platos];
        index_platos++
    ) {
        log_debug(logger_configuracion, "[CONFIG] |\t%s", cfval_platosDefault[index_platos]);
    }
    cfval_posicionRestDefaultX = config_get_int_value(config, "POSICION_REST_DEFAULT_X");
        log_debug(logger_configuracion, "[CONFIG] Pos. X restaurant default: %i", cfval_posicionRestDefaultX);
    cfval_posicionRestDefaultY = config_get_int_value(config, "POSICION_REST_DEFAULT_Y");
        log_debug(logger_configuracion, "[CONFIG] Pos. Y restaurant default: %i", cfval_posicionRestDefaultY);
}
