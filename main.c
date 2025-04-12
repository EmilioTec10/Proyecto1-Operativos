#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "CEthreads.c"  // o usá #include "CEthreads.h" si lo separás

void *print_a(void *arg) {
    for (int i = 0; i < 5; i++) {
        printf("🅰️  Hilo A, iteración %d\n", i);
        usleep(200000); // 200 ms
    }
    return NULL;
}

void *print_b(void *arg) {
    for (int i = 0; i < 5; i++) {
        printf("🅱️  Hilo B, iteración %d\n", i);
        usleep(200000); // 200 ms
    }
    return NULL;
}

int main() {
    CEthread_t thread1, thread2;

    if (CEthread_create(&thread1, print_a, NULL) != 0) {
        perror("Error creando hilo A");
        return 1;
    }

    if (CEthread_create(&thread2, print_b, NULL) != 0) {
        perror("Error creando hilo B");
        return 1;
    }

    CEthread_join(thread1);
    CEthread_join(thread2);

    printf("✅ Hilos finalizados\n");
    return 0;
}
