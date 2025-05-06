#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "planner.h"
#include "CEthreads.h"
#include "scheduler.h"

typedef struct {
    CE_Job    job;          // ya estaba
    int       done;
    int       quantum;
    int       is_rr;
    CEmutex_t start_lock;
    CEmutex_t pause_lock;
    CEmutex_t permiso_paso; // 👈 NUEVO: semáforo para autorizar cruce
} WorkerCtx;


/* -------- recurso compartido de demostración ---------------------- */
static int      g_counter = 0;
static CEmutex_t g_lock;

/* -------- función que ejecuta el hilo real ------------------------ */
static void* worker(void *arg)
{
    WorkerCtx *ctx = (WorkerCtx*)arg;

    /* ► Barrera de arranque */
    CEmutex_lock(&ctx->start_lock);
    CEmutex_unlock(&ctx->start_lock);

    while (ctx->done < ctx->job.work)
    {
        /* ► Esperar turno en RR */
        if (ctx->is_rr)
            CEmutex_lock(&ctx->pause_lock);   /* bloquea hasta que el scheduler lo suelte */

        /* ► Ejecutar un quantum ----------------------------------- */
        int slice = ctx->job.work - ctx->done;
        if (ctx->is_rr && slice > ctx->quantum)
            slice = ctx->quantum;

        for (int i = 0; i < slice; ++i) {
            CEmutex_lock(&g_lock);
            g_counter++;
            ctx->done++;
            CEmutex_unlock(&g_lock);
        }

        /* ► Avisar al scheduler y ceder el CPU -------------------- */
        if (ctx->is_rr) {
            scheduler_rr_report(getpid(), slice);

            /* Deja el cerrojo BLOQUEADO para que el scheduler
               decida cuándo volver a soltarlo */
            if (ctx->done < ctx->job.work)
                /* ya lo tenemos bloqueado: no hacemos nada */;
            else
                /* trabajo terminado → lo dejamos libre */
                    CEmutex_unlock(&ctx->pause_lock);
        }
    }

    printf("✅ Thread %d completado\n", ctx->job.id);
    return NULL;
}


/* ------------------------------------------------------------------ */
void ce_run_plan(const CE_Job jobs[], int n,
                 CE_scheduler_mode_t mode, int quantum_rr)
{
    printf("\n===== Iniciando con modo %d =====\n", mode);

    scheduler_init(mode);
    g_counter = 0;
    CEmutex_init(&g_lock);

    WorkerCtx *ctx = calloc(n, sizeof(*ctx));
    CEthread_t *thr = calloc(n, sizeof(*thr));

    /* ---- crear todos los hilos dormidos y registrarlos ------------- */
    for (int i = 0; i < n; ++i) {
        ctx[i].job     = jobs[i];
        ctx[i].done    = 0;
        ctx[i].quantum = quantum_rr;
        ctx[i].is_rr   = (mode == SCHED_CE_RR);
        // Inicialización de semáforo para permiso de paso
        CEmutex_init(&ctx[i].permiso_paso);
        CEmutex_lock(&ctx[i].permiso_paso);  // empieza bloqueado, espera luz verde


        CEmutex_init(&ctx[i].start_lock);  CEmutex_lock(&ctx[i].start_lock);
        if (ctx[i].is_rr) { CEmutex_init(&ctx[i].pause_lock);
                            CEmutex_lock(&ctx[i].pause_lock); }

        CEthread_create(&thr[i], worker, &ctx[i]);
        scheduler_add_thread(thr[i],
                             ctx[i].job.work,
                             ctx[i].job.priority,
                             ctx[i].job.deadline);
    }

    /*  ---- ciclo principal ------------------------------------------- */
    while (scheduler_has_threads()) {
        CEthread_t next = scheduler_next_thread();

        /* localizar contexto */
        WorkerCtx *c = NULL;
        for (int i = 0; i < n; ++i)
            if (thr[i].tid == next.tid) { c = &ctx[i]; break; }

        /* ► Permitir que arranque (solo la 1.ª vez) */
        CEmutex_unlock(&c->start_lock);

        if (c->is_rr) {
            /* ► Despertar un quantum */
            CEmutex_unlock(&c->pause_lock);

            /* Si acaba de terminar, esperar su fin real           */
            if (c->done >= c->job.work)
                CEthread_join(next);
        } else {
            /* Algoritmos no‑RR: esperar hasta finalizar           */
            CEthread_join(next);
        }
    }


    /* ---- resumen --------------------------------------------------- */
    int total = 0;
    for (int i = 0; i < n; ++i) total += jobs[i].work;
    printf("\n✅ Counter final = %d (esperado = %d)\n", g_counter, total);

    /* limpieza */
    for (int i = 0; i < n; ++i) {
        CEmutex_destroy(&ctx[i].start_lock);
        if (ctx[i].is_rr) CEmutex_destroy(&ctx[i].pause_lock);
    }
    CEmutex_destroy(&g_lock);
    free(ctx); free(thr);
}

