// scheduler.c
#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_THREADS 100

static ScheduledThread ready_queue[MAX_THREADS];
static int queue_size = 0;
static scheduler_algo_t current_algo;

void scheduler_init(scheduler_algo_t algo) {
    current_algo = algo;
    queue_size = 0;
    printf("[scheduler] Algoritmo inicializado: %d\n", algo);
}

void scheduler_add_thread(CEthread_t thread, int tiempo_estimado, int prioridad) {
    if (queue_size >= MAX_THREADS) {
        fprintf(stderr, "[scheduler] ¡Cola llena!\n");
        return;
    }

    ScheduledThread st;
    st.thread = thread;
    st.tiempo_estimado = tiempo_estimado;
    st.prioridad = prioridad;
    st.tiempo_llegada = time(NULL); // Usado solo para FCFS

    ready_queue[queue_size++] = st;
    printf("[scheduler] Hilo agregado a la cola. Total: %d\n", queue_size);
}

CEthread_t scheduler_next_thread() {
    if (queue_size == 0) {
        fprintf(stderr, "[scheduler] No hay hilos en cola\n");
        exit(1);
    }

    // FCFS: devolver el primer hilo en entrar
    ScheduledThread next = ready_queue[0];

    for (int i = 1; i < queue_size; ++i) {
        ready_queue[i - 1] = ready_queue[i];
    }

    queue_size--;

    return next.thread;
}

int scheduler_has_threads() {
    return queue_size > 0;
}
