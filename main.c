// main.c - Driver para el scheduler
#include <stdio.h>
#include <stdlib.h>
#include "CEthreads.h"
#include "scheduler.h"

typedef struct {
    int work;              // unidades de trabajo a ejecutar
    int id;                // id lógico solo para imprimir
    int priority;          // prioridad para el scheduler PRIORITY
    int deadline;          // parámetro de tiempo-real
    CEmutex_t start_lock;  // mutex usado como barrera de arranque
} ThreadContext;

int counter = 0;           // recurso compartido
CEmutex_t global_lock;     // protege 'counter'

// --- la función del hilo --------------------------------------------
void *run_thread(void *arg)
{
    ThreadContext *ctx = (ThreadContext *)arg;

    /* Esperar hasta que main libere el start_lock */
    CEmutex_lock(&ctx->start_lock);      // se desbloqueará desde main
    CEmutex_unlock(&ctx->start_lock);    // lo liberamos enseguida

    printf("🧵 Thread %d ejecutando %d unidades (Priority=%d, DL=%d)\n",
           ctx->id, ctx->work, ctx->priority, ctx->deadline);

    for (int i = 0; i < ctx->work; ++i) {
        CEmutex_lock(&global_lock);
        counter++;
        CEmutex_unlock(&global_lock);
    }

    printf("✅ Thread %d completado\n", ctx->id);
    return NULL;
}

void run_test(CE_scheduler_mode_t mode)
{
    /* 1. Inicializar el scheduler en el modo especificado */
    scheduler_init(mode);
    counter = 0;
    CEmutex_init(&global_lock);

    /* 2. Crear contextos con diferentes parámetros */
    ThreadContext c1 = {.work = 6, .id = 1, .priority = 3, .deadline = 10};
    ThreadContext c2 = {.work = 4, .id = 2, .priority = 5, .deadline = 5};
    ThreadContext c3 = {.work = 3, .id = 3, .priority = 2, .deadline = 7};

    /* 3. Inicializar y BLOQUEAR el start_lock de cada hilo */
    CEmutex_init(&c1.start_lock);  CEmutex_lock(&c1.start_lock);
    CEmutex_init(&c2.start_lock);  CEmutex_lock(&c2.start_lock);
    CEmutex_init(&c3.start_lock);  CEmutex_lock(&c3.start_lock);

    /* 4. Crear hilos (quedarán dormidos en su start_lock) */
    CEthread_t t1, t2, t3;
    CEthread_create(&t1, run_thread, &c1);
    CEthread_create(&t2, run_thread, &c2);
    CEthread_create(&t3, run_thread, &c3);

    /* 5. Registrar en el scheduler (aún no corren) */
    scheduler_add_thread(t1, c1.work, c1.priority, c1.deadline);
    scheduler_add_thread(t2, c2.work, c2.priority, c2.deadline);
    scheduler_add_thread(t3, c3.work, c3.priority, c3.deadline);

    printf("\n===== Iniciando ejecución con algoritmo ");
    switch(mode) {
        case SCHED_CE_FCFS: printf("FCFS =====\n"); break;
        case SCHED_CE_SJF: printf("SJF =====\n"); break;
        case SCHED_CE_PRIORITY: printf("PRIORITY =====\n"); break;
        case SCHED_CE_REALTIME: printf("REALTIME =====\n"); break;
        case SCHED_CE_RR: printf("RR =====\n"); break;
        default: printf("DESCONOCIDO =====\n");
    }

    /* 6. Bucle principal: el scheduler decide el orden */
    while (scheduler_has_threads()) {
        /* 6.1 Seleccionar el próximo hilo según el algoritmo configurado */
        CEthread_t next = scheduler_next_thread();

        /* 6.2 Determinar qué contexto corresponde */
        ThreadContext *ctx = NULL;
        if (next.tid == t1.tid) ctx = &c1;
        else if (next.tid == t2.tid) ctx = &c2;
        else if (next.tid == t3.tid) ctx = &c3;

        /* 6.3 Autorizar al hilo a correr */
        CEmutex_unlock(&ctx->start_lock);

        /* 6.4 Esperar a que termine su trabajo completo */
        CEthread_join(next);
    }

    printf("\n✅ Counter final: %d (esperado: %d)\n",
           counter, c1.work + c2.work + c3.work);

    /* Limpieza */
    CEmutex_destroy(&global_lock);
    CEmutex_destroy(&c1.start_lock);
    CEmutex_destroy(&c2.start_lock);
    CEmutex_destroy(&c3.start_lock);
}

int main(void)
{
    printf("\n======= PRUEBA DE ALGORITMOS DE PLANIFICACIÓN =======\n");

    // Prueba todos los algoritmos implementados
    // run_test(SCHED_CE_FCFS);     // First-Come-First-Served
    // run_test(SCHED_CE_SJF);      // Shortest Job First
    // run_test(SCHED_CE_PRIORITY); // Priority
     run_test(SCHED_CE_REALTIME); // Real Time
    // run_test(SCHED_CE_RR);    // Round Robin
    
    printf("\n======= PRUEBAS COMPLETADAS =======\n");
    return 0;
}