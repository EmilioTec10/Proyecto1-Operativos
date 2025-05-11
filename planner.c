#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "planner.h"
#include "CEthreads.h"
#include "scheduler.h"
#include "flow_equity.h"
#include "flow_fifo.h"

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

flow_control_mode current_flow_mode = FLOW_FIFO; // Valor por defecto

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

    switch (current_flow_mode) {
        case FLOW_EQUITY:
            equity_request_pass(ctx->from_left);
            for (int i = 0; i < ctx->job.work; ++i) {
                CEmutex_lock(&g_lock);
                g_counter++;
                avanzar_carro(ctx->job.id, i, ctx->from_left);
                CEmutex_unlock(&g_lock);
                usleep(100000);
            }
            equity_leave();
            break;
        
        case FLOW_FIFO:
            fifo_request_pass(ctx->from_left);
            for (int i = 0; i < ctx->job.work; ++i) {
                CEmutex_lock(&g_lock);
                g_counter++;
                avanzar_carro(ctx->job.id, i, ctx->from_left);
                CEmutex_unlock(&g_lock);
                usleep(100000);
            }
            fifo_leave();
            break;
        
        case FLOW_LETRERO:
            // Implementar lógica para letrero si es necesario
            break;
        
        default:
            // Lógica tradicional del scheduler (FCFS, RR, etc.)
            while (ctx->done < ctx->job.work) {
                // Para Round Robin, espera su turno
            if (ctx->is_rr)
                CEmutex_lock(&ctx->pause_lock);   /* bloquea hasta que el scheduler lo suelte */

            // Ejecuta un quantum de trabajo
            int slice = ctx->job.work - ctx->done;
            if (ctx->is_rr && slice > ctx->quantum)
                slice = ctx->quantum;

            // Hace el trabajo
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
            break;
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

    // Inicializar según el modo de flujo
    switch (current_flow_mode) {
        case FLOW_EQUITY:
            equity_init(W);
            break;
        case FLOW_FIFO:
            fifo_init();
            break;
        case FLOW_LETRERO:
            // letrero_init(); si existe
            break;
        default:
            break;
    }

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

    // Manejar la ejecución según el modo de flujo
    if (current_flow_mode == FLOW_EQUITY) {
        // Lógica existente de equity (lotes alternados)
        int lado = 1;
        while (scheduler_has_threads_left() || scheduler_has_threads_right()) {
            int lote = 0;

            // Si no hay hilos del lado actual, cambia de lado
            if ((lado == 1 && !scheduler_has_threads_left()) ||
                (lado == 0 && !scheduler_has_threads_right())) {
                // No hay hilos del lado actual → cambiar de lado
                lado = 1 - lado;
                continue;
            }
        
            // Ejecutar lote de W hilos del lado actual
            while (lote < W) {
                // Si no hay más hilos de este lado, sale
                if ((lado == 1 && !scheduler_has_threads_left()) ||
                    (lado == 0 && !scheduler_has_threads_right())) break;
                
                // Obtiene el próximo hilo del lado actual
                CEthread_t next = (lado == 1)
                    ? scheduler_next_thread_from_left()
                    : scheduler_next_thread_from_right();

                // Busca el contexto correspondiente
                WorkerCtx *c = NULL;
                for (int i = 0; i < n; ++i)
                    if (thr[i].tid == next.tid) { c = &ctx[i]; break; }

                // Si encuentra el contexto y no ha terminado, lo ejecuta
                if (c && !c->terminado) {
                    CEmutex_unlock(&c->start_lock); // Desbloquea el hilo
                    CEthread_join(next);            // Espera a que termine
                    c->terminado = 1;
                    lote++;                         // Cuenta este hilo en el lote
                }
            }
        
            lado = 1 - lado;  // Alterna el lado para el próximo lote
        }
    } else if (current_flow_mode == FLOW_FIFO) {
        // Solo se unen los hilos, FIFO se maneja internamente
        for (int i = 0; i < n; ++i) {
            CEmutex_unlock(&ctx[i].start_lock);
            CEthread_join(thr[i]);
        }
    } else if (current_flow_mode == FLOW_LETRERO) {
        // Lógica para letrero
    } else {
        // Lógica tradicional del scheduler
        while (scheduler_has_threads()) {
            // Determinar lado en base al contenido de las colas
            CEthread_t next;
            
            // Selecciona el próximo hilo según el algoritmo
            if (scheduler_has_threads()) {
                if (mode == SCHED_CE_SJF || mode == SCHED_CE_FCFS || mode == SCHED_CE_PRIORITY || mode == SCHED_CE_REALTIME) {
                    // Intenta primero con la izquierda
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

            // Busca el contexto del hilo seleccionado
            WorkerCtx *c = NULL;
            for (int i = 0; i < n; ++i)
                if (thr[i].tid == next.tid) { c = &ctx[i]; break; }

            // Desbloquea el hilo para que ejecute
            CEmutex_unlock(&c->start_lock);

            // Para Round Robin
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
