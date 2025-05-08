#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "planner.h"
#include "CEthreads.h"
#include "scheduler.h"
#include "flow_equity.h"

typedef struct {
    CE_Job    job;          // ya estaba
    int       done;
    int       quantum;
    int       is_rr;
    int from_left; // 1 si viene de la izquierda, 0 si de la derecha
    int terminado; 
    CEmutex_t start_lock;
    CEmutex_t pause_lock;
    CEmutex_t permiso_paso; // 👈 NUEVO: semáforo para autorizar cruce
} WorkerCtx;

int modo_flujo_equity = 1; // o 0, según el algoritmo de flujo elegido

/* -------- recurso compartido de demostración ---------------------- */
static int      g_counter = 0;
static CEmutex_t g_lock;
static CEmutex_t print_lock;

void avanzar_carro(int id, int paso, int from_left) {
    const char* lado = from_left ? "Izquierda" : "Derecha";

    CEmutex_lock(&print_lock);
    printf("🚗 Carro %d (%s) avanzando paso %d\n", id, lado, paso + 1);
    CEmutex_unlock(&print_lock);
}

/* -------- función que ejecuta el hilo real ------------------------ */
static void* worker(void *arg)
{
    WorkerCtx *ctx = (WorkerCtx*)arg;

    /* ► Barrera de arranque */
    CEmutex_lock(&ctx->start_lock);
    CEmutex_unlock(&ctx->start_lock);

    CEmutex_lock(&print_lock);
    printf("🚦 [TID %d] Hilo creado. ID = %d, Lado = %s\n", getpid(), ctx->job.id,
        ctx->from_left ? "Izquierda" : "Derecha");
    CEmutex_unlock(&print_lock);

    if (modo_flujo_equity) {
        // 🟢 Algoritmo de flujo: Equidad
        equity_request_pass(ctx->from_left);

        for (int i = 0; i < ctx->job.work; ++i) {
            CEmutex_lock(&g_lock);
            g_counter++;
            avanzar_carro(ctx->job.id, i, ctx->from_left);  // Podés personalizar esta función
            CEmutex_unlock(&g_lock);
            usleep(100000);  // Velocidad de cruce
        }

        equity_leave();
    } else {
        // 🔵 Algoritmo de planificación tradicional (FCFS, RR, etc.)
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

                if (ctx->done < ctx->job.work)
                    ; // seguimos bloqueados hasta que el scheduler nos suelte
                else
                    CEmutex_unlock(&ctx->pause_lock);
            }
        }
    }

    CEmutex_lock(&print_lock);
    printf("✅ Thread %d completado\n", ctx->job.id);
    CEmutex_unlock(&print_lock);

    return NULL;
}



/* ------------------------------------------------------------------ */
void ce_run_plan(const CE_Job jobs[], int n,
                 CE_scheduler_mode_t mode, int quantum_rr, int W)
{
    printf("\n===== Iniciando con modo %d =====\n", mode);

    scheduler_init(mode);
    g_counter = 0;
    CEmutex_init(&g_lock);
    CEmutex_init(&print_lock);

    WorkerCtx *ctx = calloc(n, sizeof(*ctx));
    CEthread_t *thr = calloc(n, sizeof(*thr));

    if (modo_flujo_equity) {
        equity_init(W);
    }

    /* ---- crear todos los hilos dormidos y registrarlos ------------- */
    for (int i = 0; i < n; ++i) {
        ctx[i].job        = jobs[i];
        ctx[i].done       = 0;
        ctx[i].quantum    = quantum_rr;
        ctx[i].is_rr      = (mode == SCHED_CE_RR);
        ctx[i].from_left  = jobs[i].from_left;
        ctx[i].terminado  = 0;

        CEmutex_init(&ctx[i].permiso_paso);
        CEmutex_lock(&ctx[i].permiso_paso);

        CEmutex_init(&ctx[i].start_lock);
        CEmutex_lock(&ctx[i].start_lock);

        if (ctx[i].is_rr) {
            CEmutex_init(&ctx[i].pause_lock);
            CEmutex_lock(&ctx[i].pause_lock);
        }

        printf("[DEBUG] Creando hilo %d (ID %d, from_left = %d)\n",
               i, jobs[i].id, jobs[i].from_left);

        CEthread_create(&thr[i], worker, &ctx[i]);

        scheduler_add_thread(thr[i],
                             ctx[i].job.work,
                             ctx[i].job.priority,
                             ctx[i].job.deadline);
    }

    if (modo_flujo_equity) {
        // --- NUEVO: planificación por lote por lado ---
        int lado = 1; // 1 = izquierda, 0 = derecha
        int hilos_terminados = 0;

        while (hilos_terminados < n) {
            int lote = 0;

            for (int i = 0; i < n && lote < W; ++i) {
                if (!ctx[i].terminado && ctx[i].from_left == lado) {
                    CEmutex_unlock(&ctx[i].start_lock);
                    CEthread_join(thr[i]);
                    ctx[i].terminado = 1;
                    hilos_terminados++;
                    lote++;
                }
            }

            lado = 1 - lado;
        }

    } else {
        // 🔁 lógica original para otros algoritmos
        while (scheduler_has_threads()) {
            CEthread_t next = scheduler_next_thread();

            WorkerCtx *c = NULL;
            for (int i = 0; i < n; ++i)
                if (thr[i].tid == next.tid) { c = &ctx[i]; break; }

            CEmutex_unlock(&c->start_lock);

            if (c->is_rr) {
                CEmutex_unlock(&c->pause_lock);
                if (c->done >= c->job.work)
                    CEthread_join(next);
            } else {
                CEthread_join(next);
            }
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
    CEmutex_destroy(&print_lock);

    free(ctx); free(thr);
}


