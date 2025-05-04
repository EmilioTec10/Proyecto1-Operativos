// scheduler.c
#include "scheduler.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define MAX_THREADS 100

static ScheduledThread ready_queue[MAX_THREADS];
static int queue_size = 0;
static CE_scheduler_mode_t current_mode;

void scheduler_init(CE_scheduler_mode_t mode) {
    current_mode = mode;
    queue_size = 0;
    printf("[scheduler] Algoritmo inicializado: ");

    switch(mode) {
        case SCHED_CE_FCFS:
            printf("First-Come-First-Served (FCFS)\n");
            break;
        case SCHED_CE_SJF:
            printf("Shortest Job First (SJF)\n");
            break;
        case SCHED_CE_PRIORITY:
            printf("Priority\n");
            break;
        case SCHED_CE_RR:
            printf("Round Robin (RR)\n");
            break;
        case SCHED_CE_REALTIME:
            printf("Real Time\n");
            break;
        default:
            printf("Desconocido\n");
    }
}

void scheduler_add_thread(CEthread_t thread, int estimated_time, int priority, int deadline) {
    if (queue_size >= MAX_THREADS) {
        fprintf(stderr, "[scheduler] ¡Cola llena!\n");
        return;
    }

    ScheduledThread st;
    st.thread = thread;
    st.estimated_time = estimated_time;
    st.priority = priority;
    st.arrival_time = time(NULL);  // usado para FCFS y tiempo real
    st.remaining_work = estimated_time;  // usado para RR
    st.deadline = deadline;  // usado para tiempo real

    ready_queue[queue_size++] = st;
    printf("[scheduler] Hilo agregado a la cola. Total: %d\n", queue_size);
}

CEthread_t scheduler_next_thread() {
    if (queue_size == 0) {
        fprintf(stderr, "[scheduler] No hay hilos en cola\n");
        exit(1);
    }

    int selected_index = 0;

    switch (current_mode) {
        case SCHED_CE_FCFS:
            // FCFS: el que llegó primero (índice 0 en la cola)
            selected_index = 0;
            printf("[scheduler] FCFS seleccionó el hilo más antiguo\n");
            break;

        case SCHED_CE_SJF:
            // SJF: el que tenga menor estimated_time
            {
                int min_time = ready_queue[0].estimated_time;
                for (int i = 1; i < queue_size; ++i) {
                    if (ready_queue[i].estimated_time < min_time) {
                        min_time = ready_queue[i].estimated_time;
                        selected_index = i;
                    }
                }
                printf("[scheduler] SJF seleccionó hilo con tiempo estimado: %d\n",
                       ready_queue[selected_index].estimated_time);
            }
            break;

        case SCHED_CE_PRIORITY:
            // Priority: el que tenga mayor prioridad
            {
                int max_priority = ready_queue[0].priority;
                for (int i = 1; i < queue_size; ++i) {
                    if (ready_queue[i].priority > max_priority) {
                        max_priority = ready_queue[i].priority;
                        selected_index = i;
                    }
                }
                printf("[scheduler] PRIORITY seleccionó hilo con prioridad: %d\n",
                       ready_queue[selected_index].priority);
            }
            break;

        case SCHED_CE_REALTIME:
            // Real-time: el que tenga el deadline más cercano
            {
                int min_deadline = ready_queue[0].deadline;
                for (int i = 1; i < queue_size; ++i) {
                    if (ready_queue[i].deadline < min_deadline) {
                        min_deadline = ready_queue[i].deadline;
                        selected_index = i;
                    }
                }
                printf("[scheduler] REALTIME seleccionó hilo con deadline: %d\n",
                       ready_queue[selected_index].deadline);
            }
            break;

        case SCHED_CE_RR:
            /* Round Robin: tomar el primero de la cola */
                selected_index = 0;
        printf("[scheduler] RR seleccionó hilo con %d unidades restantes\n",
               ready_queue[0].remaining_work);
        break;
        default:
            fprintf(stderr, "[scheduler] Algoritmo desconocido.\n");
        exit(1);
    }

    ScheduledThread sel = ready_queue[selected_index];

    /* --- lógica específica RR ------------------------------------ */
    if (current_mode == SCHED_CE_RR) {

        /* si aún queda trabajo después del aviso del hilo… */
        if (sel.remaining_work > 0) {
            /* mover al final de la cola */
            for (int i = selected_index; i < queue_size - 1; ++i)
                ready_queue[i] = ready_queue[i + 1];
            ready_queue[queue_size - 1] = sel;
            /* queue_size no cambia */
        } else {
            /* terminó: quitar definitivamente */
            for (int i = selected_index; i < queue_size - 1; ++i)
                ready_queue[i] = ready_queue[i + 1];
            queue_size--;
        }
    }
    else {
        /* modos que no son RR: quitar de la cola */
        for (int i = selected_index; i < queue_size - 1; ++i)
            ready_queue[i] = ready_queue[i + 1];
        queue_size--;
    }
    /* -------------------------------------------------------------- */

    return sel.thread;
}

int scheduler_has_threads() {
    return queue_size > 0;
}

/* ---- SOLO PARA Round‑Robin -------------------------------------- */
void scheduler_rr_report(pid_t tid, int unidades)
{
    if (current_mode != SCHED_CE_RR) return;

    for (int i = 0; i < queue_size; ++i) {
        if (ready_queue[i].thread.tid == tid) {
            ready_queue[i].remaining_work -= unidades;
            break;
        }
    }
}