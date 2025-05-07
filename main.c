#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "CEthreads.h"
#include "flow_equity.h"

#define NUM_LEFT  3
#define NUM_RIGHT 3
#define W         2
#define DISTANCE  5

typedef struct {
    int id;
    int from_left; // 1 si viene de la izquierda, 0 si de la derecha
} Carro;

CEmutex_t print_lock;

void avanzar_carro(int id, int paso, int from_left) {
    CEmutex_lock(&print_lock);
    const char* lado = from_left ? "Izquierda" : "Derecha";
    printf("🚗 Carro %d (%s) avanzando paso %d\n", id, lado, paso + 1);
    CEmutex_unlock(&print_lock);
}


void* carro_thread(void* arg) {
    if (arg == NULL) {
        fprintf(stderr, "❌ Error: carro_thread recibió NULL\n");
        return NULL;
    }

    Carro* c = (Carro*)arg;

    equity_request_pass(c->from_left);

    for (int i = 0; i < DISTANCE; ++i) {
        avanzar_carro(c->id, i, c->from_left);
        usleep(100000);
    }

    equity_leave();
    CEmutex_lock(&print_lock);
    printf("✅ Carro %d terminó de cruzar\n", c->id);
    CEmutex_unlock(&print_lock);
    return NULL;
}


int main() {
    equity_init(W);
    
    CEmutex_init(&print_lock);

    CEthread_t threads[NUM_LEFT + NUM_RIGHT];
    Carro* carros[NUM_LEFT + NUM_RIGHT];

    // Crear carros de la izquierda
    for (int i = 0; i < NUM_LEFT; ++i) {
        carros[i] = malloc(sizeof(Carro));
        carros[i]->id = i;
        carros[i]->from_left = 1; // o 0
        CEthread_create(&threads[i], carro_thread, carros[i]);

    }

    // Crear carros de la derecha
    for (int i = 0; i < NUM_RIGHT; ++i) {
        carros[NUM_LEFT + i] = malloc(sizeof(Carro));
        carros[NUM_LEFT + i]->id = NUM_LEFT + i;
        carros[NUM_LEFT + i]->from_left = 0;
        CEthread_create(&threads[NUM_LEFT + i], carro_thread, carros[NUM_LEFT + i]);
    }

    // Esperar a que terminen todos
    for (int i = 0; i < NUM_LEFT + NUM_RIGHT; ++i) {
        CEthread_join(threads[i]);
        free(carros[i]);  // ← solo después de join
    }
    
    CEmutex_destroy(&print_lock);


    printf("🎉 Todos los carros han cruzado.\n");
    return 0;
}
