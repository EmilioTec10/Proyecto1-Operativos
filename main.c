#include <stdio.h>
#include <unistd.h>
#include "CEthreads.c" // o compílalo por separado

void *hello(void *arg) {
    int id = *(int *)arg;
    printf("Hola desde el hilo %d (PID %d)\n", id, getpid());
    return NULL;
}

int main() {
    CEthread_t t1;
    int id = 1;

    if (CEthread_create(&t1, hello, &id) != 0) {
        fprintf(stderr, "Error creando CEthread\n");
        return 1;
    }

    sleep(1); // para que el hilo tenga tiempo de terminar
    printf("Hilo creado con tid = %d\n", t1.tid);
    return 0;
}
