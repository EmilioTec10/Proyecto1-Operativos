#ifndef ROAD_H
#define ROAD_H

#include "CEthreads.h"

// Definición de los lados de la carretera
typedef enum { LEFT_SIDE, RIGHT_SIDE } Side;

// Estructura que representa el estado de la carretera
typedef struct {
    int W;                        // Número máximo de carros que pueden cruzar de un lado
    int counter;                  // Número de carros que han cruzado de un lado
    int cars_crossing;            // Número de carros actualmente cruzando
    Side current_direction;       // Lado actual permitido para cruzar
    int waiting[2];               // Número de carros esperando en cada lado
    int cars_crossing_side;       // Lado de los carros que están cruzando
    CEmutex_t lock;               // Mutex para proteger el estado de la carretera
    CEmutex_t cond_var;           // Mutex para condiciones de espera
} Road;

// Funciones relacionadas con la carretera

// Inicializa la carretera
void road_init(Road *r, int W);

// Libera los recursos de la carretera
void road_destroy(Road *r);

// Función bloqueante. El hilo espera hasta que pueda cruzar la carretera
void road_wait_for_turn(Road *r, Side side);

// Llamada cuando el hilo termina de cruzar. Libera espacio y actualiza el sentido
void road_exit(Road *r, Side side);

// Función para gestionar el cruce de los carros
void road_cross(Road *r, Side side);

#endif // ROAD_H
