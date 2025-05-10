#include <stdio.h>
#include <stdlib.h>
#include "CEthreads.c"  // Incluye todas tus funciones

#define N 100000

int contador = 0;
CEmutex_t lock;

void *incrementar(void *arg) {
    for (int i = 0; i < N; i++) {
        CEmutex_lock(&lock);
        contador++;
        CEmutex_unlock(&lock);
    }
    return NULL;
}

int main() {
    CEthread_t t1, t2;

    // Inicializar el mutex
    CEmutex_init(&lock);

    // Crear dos hilos
    CEthread_create(&t1, incrementar, NULL);
    CEthread_create(&t2, incrementar, NULL);

    // Esperar a que terminen
    CEthread_join(t1);
    CEthread_join(t2);

    // Mostrar resultado final
    printf("✅ Contador final: %d (esperado: %d)\n", contador, 2 * N);

    // Destruir el mutex
    if (CEmutex_destroy(&lock) != 0) {
        fprintf(stderr, "❌ Error al destruir el mutex\n");
    }

    return 0;
}