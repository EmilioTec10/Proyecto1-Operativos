#include <stdio.h>
#include <stdlib.h>
#include "CEthreads.h"
#include "scheduler.h"

#define N 5

int contador = 0;
CEmutex_t lock;

void *incrementar(void *arg) {
    printf("🧵 Hilo comenzando...\n");
    for (int i = 0; i < N; i++) {
        CEmutex_lock(&lock);
        contador++;
        CEmutex_unlock(&lock);
    }
    printf("✅ Hilo terminó\n");
    return NULL;
}
int main() {
    scheduler_init(SCHED_ALGO_FCFS); // Inicializamos el algoritmo

    CEthread_t t1, t2;

    CEmutex_init(&lock);

    // Creamos los hilos pero los mandamos al planificador
    CEthread_create(&t1, incrementar, NULL);
    scheduler_add_thread(t1, 0, 0);  // FCFS → no importa el orden

    CEthread_create(&t2, incrementar, NULL);
    scheduler_add_thread(t2, 0, 0);

    // Ahora los sacamos de la cola en orden
    while (scheduler_has_threads()) {
        CEthread_t next = scheduler_next_thread();
        CEthread_join(next);  // Ejecutamos y esperamos al hilo real
    }

    printf("✅ Contador final: %d (esperado: %d)\n", contador, 2 * N);

    if (CEmutex_destroy(&lock) != 0) {
        fprintf(stderr, "❌ Error al destruir el mutex\n");
    }

    return 0;
}
