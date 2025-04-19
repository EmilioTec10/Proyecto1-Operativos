// main.c - Driver para el scheduler
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "CEthreads.h"
#include "scheduler.h"

#define QUANTUM 5

typedef struct {
    int work;
    int id;
    int priority;
    int deadline;
    int work_done;
    int is_rr_mode;
    CEmutex_t start_lock;
    CEmutex_t pause_lock;  // Solo usado en RR
} ThreadContext;

int counter = 0;
CEmutex_t global_lock;

void *run_thread(void *arg)
{
    ThreadContext *ctx = (ThreadContext *)arg;

    /* esperar señal de inicio */
    CEmutex_lock(&ctx->start_lock);
    CEmutex_unlock(&ctx->start_lock);

    while (ctx->work_done < ctx->work) {

        /* --- ejecutar UN quantum o lo que falte --- */
        int unidades = (ctx->work - ctx->work_done < QUANTUM)
                       ? (ctx->work - ctx->work_done)
                       : QUANTUM;

        for (int i = 0; i < unidades; ++i) {
            CEmutex_lock(&global_lock);
            counter++;
            ctx->work_done++;
            CEmutex_unlock(&global_lock);
        }

        /* avisar al scheduler cuántas unidades hicimos en este turno (solo RR) */
        if (ctx->is_rr_mode)
            scheduler_rr_report(getpid(), unidades);

        /* si queda trabajo y esto es RR, pausar hasta el siguiente turno */
        if (ctx->is_rr_mode && ctx->work_done < ctx->work)
            CEmutex_lock(&ctx->pause_lock);
    }

    printf("✅ Thread %d completado\n", ctx->id);
    return NULL;
}


void run_test(CE_scheduler_mode_t mode)
{
    scheduler_init(mode);
    counter = 0;
    CEmutex_init(&global_lock);

    ThreadContext c1 = {.work = 66, .id = 1, .priority = 3, .deadline = 10};
    ThreadContext c2 = {.work = 19, .id = 2, .priority = 5, .deadline = 5};
    ThreadContext c3 = {.work = 10, .id = 3, .priority = 2, .deadline = 7};

    ThreadContext *contexts[] = {&c1, &c2, &c3};
    CEthread_t threads[3];

    for (int i = 0; i < 3; ++i) {
        contexts[i]->work_done = 0;
        contexts[i]->is_rr_mode = (mode == SCHED_CE_RR);
        CEmutex_init(&contexts[i]->start_lock);
        CEmutex_lock(&contexts[i]->start_lock);
        if (mode == SCHED_CE_RR) {
            CEmutex_init(&contexts[i]->pause_lock);
            CEmutex_lock(&contexts[i]->pause_lock);
        }
        CEthread_create(&threads[i], run_thread, contexts[i]);
        scheduler_add_thread(threads[i], contexts[i]->work, contexts[i]->priority, contexts[i]->deadline);
    }

    printf("\n===== Iniciando ejecución con algoritmo %s =====\n",
           mode == SCHED_CE_RR ? "RR" : "Otro");

    while (scheduler_has_threads()) {
        CEthread_t next = scheduler_next_thread();
        ThreadContext *ctx = NULL;
        for (int i = 0; i < 3; ++i) {
            if (threads[i].tid == next.tid) {
                ctx = contexts[i];
                break;
            }
        }
        CEmutex_unlock(&ctx->start_lock);
        if (mode == SCHED_CE_RR && ctx->work_done < ctx->work) {
            CEmutex_unlock(&ctx->pause_lock);
        } else {
            CEthread_join(next);
        }
    }

    printf("\n✅ Counter final: %d (esperado: %d)\n",
           counter, c1.work + c2.work + c3.work);

    CEmutex_destroy(&global_lock);
    for (int i = 0; i < 3; ++i) {
        CEmutex_destroy(&contexts[i]->start_lock);
        if (mode == SCHED_CE_RR) CEmutex_destroy(&contexts[i]->pause_lock);
    }
}

int main(void)
{
    printf("\n======= PRUEBA DE ALGORITMOS DE PLANIFICACIÓN =======\n");
    // run_test(SCHED_CE_FCFS);
    // run_test(SCHED_CE_SJF);
    // run_test(SCHED_CE_PRIORITY);
    // run_test(SCHED_CE_REALTIME);
     run_test(SCHED_CE_RR);
    printf("\n======= PRUEBAS COMPLETADAS =======\n");
    return 0;
}
