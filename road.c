#include "road.h"
#include <stdio.h>

// Inicializa la carretera
void road_init(Road *r, int W) {
    // Inicializamos el mutex para proteger el estado de la carretera
    CEmutex_init(&r->lock);
    CEmutex_init(&r->cond_var);  // Usamos este mutex como una simulación de condición

    // Configuramos las variables de la carretera
    r->W = W;                      // Establece el número máximo de carros que pueden cruzar
    r->counter = 0;                 // Inicializa el contador de carros cruzados a 0
    r->cars_crossing = 0;           // Ningún carro está cruzando al principio
    r->current_direction = LEFT_SIDE;  // Inicializa el sentido a izquierda

    r->waiting[LEFT_SIDE] = 0;      // No hay carros esperando en el lado izquierdo
    r->waiting[RIGHT_SIDE] = 0;     // No hay carros esperando en el lado derecho
    r->cars_crossing_side = LEFT_SIDE;  // Por defecto, los carros cruzan desde la izquierda
}

// Libera los recursos de la carretera
void road_destroy(Road *r) {
    // Destruimos los mutex utilizados para la sincronización
    CEmutex_destroy(&r->lock);
    CEmutex_destroy(&r->cond_var);
}

// Función bloqueante. El hilo espera hasta que pueda cruzar la carretera
void road_wait_for_turn(Road *r, Side side) {
    int opposite = (side == LEFT_SIDE) ? RIGHT_SIDE : LEFT_SIDE;

    while (1) {
        CEmutex_lock(&r->lock);  // Protegemos el acceso a las variables compartidas

        // Incrementamos el número de carros esperando en el lado correspondiente
        r->waiting[side]++;

        // Verificamos si el carro puede cruzar:
        // - El lado actual debe ser el permitido para cruzar
        // - El número de carros que han cruzado debe ser menor a W
        // - No deben haber carros cruzando de otro lado, o si los hay, deben ser del mismo lado
        int can_cross = (r->current_direction == side) && 
                        (r->counter < r->W) && 
                        (r->cars_crossing == 0 || r->cars_crossing_side == side);

        // Si no hay carros esperando en el lado opuesto, cambiamos la dirección de cruce
        if (r->waiting[r->current_direction] == 0) {
            r->current_direction = opposite;
            r->counter = 0;  // Reseteamos el contador de carros que han cruzado
        }

        // Si el carro puede cruzar, actualizamos las variables y permitimos que pase
        if (can_cross) {
            r->waiting[side]--;           // Decrementamos los carros esperando en el lado
            r->cars_crossing++;           // Aumentamos el número de carros cruzando
            r->cars_crossing_side = side; // Establecemos el lado de los carros cruzando
            r->counter++;                 // Aumentamos el contador de carros que han cruzado
            CEmutex_unlock(&r->lock);     // Liberamos el mutex y permitimos que el carro pase
            break;                        // El carro ya puede cruzar, salimos del bucle
        }

        CEmutex_unlock(&r->lock);  // Si no puede cruzar, liberamos el mutex y esperamos
        CEthread_sleep(1);         // Espera activa simple (simula un tiempo de espera)
    }
}

// Llamada cuando el hilo termina de cruzar. Libera espacio y actualiza el sentido
void road_exit(Road *r, Side side) {
    int opposite = (side == LEFT_SIDE) ? RIGHT_SIDE : LEFT_SIDE;

    // Bloqueamos el acceso para actualizar el estado de la carretera
    CEmutex_lock(&r->lock);

    // Decrementamos el número de carros cruzando
    r->cars_crossing--;

    // Si se ha alcanzado el límite de carros (W) o no hay más carros esperando en el lado,
    // cambiamos el sentido de la carretera y reseteamos el contador
    if (r->counter >= r->W || r->waiting[side] == 0) {
        r->current_direction = opposite;  // Cambiamos la dirección del cruce
        r->counter = 0;                  // Reseteamos el contador de carros cruzados
    }

    CEmutex_unlock(&r->lock);  // Liberamos el acceso a las variables compartidas
}

// Función para gestionar el cruce de los carros
void road_cross(Road *r, Side side) {
    int opposite = (side == LEFT_SIDE) ? RIGHT_SIDE : LEFT_SIDE;

    while (1) {
        // Bloqueamos el acceso para actualizar el estado de la carretera
        CEmutex_lock(&r->lock);

        // Incrementamos el número de carros esperando en el lado correspondiente
        r->waiting[side]++;

        // Verificamos si el carro puede cruzar:
        // - El lado actual debe ser el permitido para cruzar
        // - El número de carros que han cruzado debe ser menor a W
        // - No deben haber carros cruzando de otro lado, o si los hay, deben ser del mismo lado
        int can_cross = (r->current_direction == side) &&
                        (r->counter < r->W) &&
                        (r->cars_crossing == 0 || r->cars_crossing_side == side);

        // Si no hay carros esperando en el lado opuesto, cambiamos la dirección de cruce
        if (r->waiting[r->current_direction] == 0) {
            r->current_direction = opposite;
            r->counter = 0;  // Reseteamos el contador de carros cruzados
        }

        // Si el carro puede cruzar, actualizamos las variables y permitimos que pase
        if (can_cross) {
            r->waiting[side]--;           // Decrementamos los carros esperando en el lado
            r->cars_crossing++;           // Aumentamos el número de carros cruzando
            r->cars_crossing_side = side; // Establecemos el lado de los carros cruzando
            r->counter++;                 // Aumentamos el contador de carros que han cruzado
            CEmutex_unlock(&r->lock);     // Liberamos el mutex y permitimos que el carro pase
            break;                        // El carro ya puede cruzar, salimos del bucle
        }

        CEmutex_unlock(&r->lock);  // Si no puede cruzar, liberamos el mutex y esperamos
        CEthread_sleep(1);         // Espera activa simple (simula un tiempo de espera)
    }
}

