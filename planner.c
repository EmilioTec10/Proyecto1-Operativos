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
         // 🟢 Control de flujo por equidad
        equity_request_pass(ctx->from_left, ctx->is_rr);

        if (ctx->is_rr) {
            while (ctx->done < ctx->job.work) {
                CEmutex_lock(&ctx->pause_lock);  // Espera su turno RR

                int slice = ctx->job.work - ctx->done;
                if (slice > ctx->quantum)
                    slice = ctx->quantum;

                for (int i = 0; i < slice; ++i) {
                    CEmutex_lock(&g_lock);
                    avanzar_carro(ctx->job.id, ctx->done, ctx->from_left);
                    ctx->done++;
                    g_counter++;
                    CEmutex_unlock(&g_lock);
                    usleep(100000);
                }

                scheduler_rr_report(getpid(), slice);

                // Si terminó, salir del ciclo
                if (ctx->done >= ctx->job.work)
                    break;
            }
        } else {
            for (int i = 0; i < ctx->job.work; ++i) {
                CEmutex_lock(&g_lock);
                avanzar_carro(ctx->job.id, i, ctx->from_left);
                g_counter++;
                CEmutex_unlock(&g_lock);
                usleep(100000);
            }
        }

        equity_leave(ctx->is_rr);
    } else {
        // 🔵 Algoritmo de planificación tradicional (FCFS, RR, etc.)
        while (ctx->done < ctx->job.work)
        {
            /* ► Esperar turno en RR */
            if (ctx->is_rr){
                CEmutex_lock(&ctx->pause_lock);   /* bloquea hasta que el scheduler lo suelte */
            }
               

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
                 CE_scheduler_mode_t mode, int quantum_rr, int W) {

    printf("\n===== Iniciando con modo %d =====\n", mode);

    scheduler_init(mode);

    g_counter = 0;
    CEmutex_init(&g_lock);
    CEmutex_init(&print_lock);

    WorkerCtx *ctx = calloc(n, sizeof(*ctx));
    CEthread_t *thr = calloc(n, sizeof(*thr));

    if (modo_flujo_equity)
        equity_init(W);

    for (int i = 0; i < n; ++i) {
        ctx[i].job = jobs[i];
        ctx[i].done = 0;
        ctx[i].quantum = quantum_rr;
        ctx[i].is_rr = (mode == SCHED_CE_RR);
        ctx[i].from_left = jobs[i].from_left;
        ctx[i].terminado = 0;

        CEmutex_init(&ctx[i].start_lock);
        CEmutex_lock(&ctx[i].start_lock);

        if (ctx[i].is_rr) {
            CEmutex_init(&ctx[i].pause_lock);
            CEmutex_lock(&ctx[i].pause_lock);
        }

        CEthread_create(&thr[i], worker, &ctx[i]);

        scheduler_add_thread(thr[i],
                             ctx[i].job.work,
                             ctx[i].job.priority,
                             ctx[i].job.deadline,
                             ctx[i].from_left);
    }

    if (modo_flujo_equity) {
        int lado = 1; // 1 = izquierda, 0 = derecha
        int terminados = 0;

        while (scheduler_has_threads_left() || scheduler_has_threads_right()) {
            int lote = 0;
        
            if ((lado == 1 && !scheduler_has_threads_left()) ||
                (lado == 0 && !scheduler_has_threads_right())) {
                // No hay hilos del lado actual → cambiar de lado
                lado = 1 - lado;
                continue;
            }
        
            // Ejecutar lote de W hilos del lado actual
            while (lote < W) {
                printf("tiene threads %d\n", scheduler_has_threads_left());
                int pendientes = 0;
                for (int i = 0; i < n; ++i) {
                    if (ctx[i].from_left == lado && !ctx[i].terminado)
                        pendientes++;
                }
                if (pendientes == 0) break;
        
                CEthread_t next = (lado == 1)
                    ? scheduler_next_thread_from_left()
                    : scheduler_next_thread_from_right();
        
                WorkerCtx *c = NULL;
                for (int i = 0; i < n; ++i)
                    if (thr[i].tid == next.tid) { c = &ctx[i]; break; }
        
                if (c && !c->terminado) {
                    CEmutex_unlock(&c->start_lock); // solo al inicio

                    if (c->is_rr) {
                        // 🔓 Desbloquea solo su quantum
                        printf("🟢 Desbloqueando hilo ID=%d (TID=%d) para ejecutar quantum\n", c->job.id, next.tid);
                        CEmutex_unlock(&c->pause_lock);

                        usleep(quantum_rr * 100000); // Espera a que termine su quantum

                        if (c->done >= c->job.work) {
                            CEthread_join(next);
                            c->terminado = 1;
                            lote++;
                        } else {
                        // 🔁 Reinsertar en el scheduler para el próximo turno
                        scheduler_add_thread(next,
                                            c->job.work,
                                            c->job.priority,
                                            c->job.deadline,
                                            c->from_left);
                        }
                    } else {
                        CEthread_join(next);
                        c->terminado = 1;
                        lote++;
                    }
                }
            }
        
            lado = 1 - lado;  // Alternar dirección
        }
        

    } else {
        while (scheduler_has_threads()) {
            // Determinar lado en base al contenido de las colas
            CEthread_t next;
            if (scheduler_has_threads()) {
                if (mode == SCHED_CE_SJF || mode == SCHED_CE_FCFS || mode == SCHED_CE_PRIORITY || mode == SCHED_CE_REALTIME) {
                    // Default a la izquierda primero si existen elementos
                    if (scheduler_has_threads()) {
                        if ((next = scheduler_next_thread_from_left()).tid != 0) {
                            ;
                        } else {
                            next = scheduler_next_thread_from_right();
                        }
                    }
                } else {
                    // En RR o cualquier otro, se puede extender a soporte dual RR
                    fprintf(stderr, "[scheduler] Modo no soportado en dual-cola\n");
                    exit(1);
                }
            } else break;

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

    int total = 0;
    for (int i = 0; i < n; ++i) total += jobs[i].work;
    printf("\n✅ Counter final = %d (esperado = %d)\n", g_counter, total);

    for (int i = 0; i < n; ++i) {
        CEmutex_destroy(&ctx[i].start_lock);
        if (ctx[i].is_rr) CEmutex_destroy(&ctx[i].pause_lock);
    }
    CEmutex_destroy(&g_lock);
    CEmutex_destroy(&print_lock);

    free(ctx);
    free(thr);
}
