#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define MAX_THREADS 100

static ScheduledThread ready_left[MAX_THREADS];
static ScheduledThread ready_right[MAX_THREADS];
static int size_left = 0;
static int size_right = 0;

static CE_scheduler_mode_t current_mode;
static CEmutex_t sched_lock;

void scheduler_init(CE_scheduler_mode_t mode) {
    current_mode = mode;
    size_left = size_right = 0;
    CEmutex_init(&sched_lock);

    printf("[scheduler] Algoritmo inicializado: ");
    switch (mode) {
        case SCHED_CE_FCFS:     puts("FCFS");        break;
        case SCHED_CE_SJF:      puts("SJF");         break;
        case SCHED_CE_PRIORITY: puts("PRIORITY");    break;
        case SCHED_CE_REALTIME: puts("REALTIME");    break;
        case SCHED_CE_RR:       puts("Round Robin"); break;
        default:                puts("Desconocido");
    }
}

void scheduler_add_thread(CEthread_t th, int est_time, int prio, int deadline, int from_left) {
    CEmutex_lock(&sched_lock);

    printf("[scheduler] Hilo ID=%d agregado al lado %s. Total=%d\n",
        th.tid, from_left ? "Izquierda" : "Derecha", from_left ? size_left : size_right);
 

    ScheduledThread st = {
        .thread = th,
        .estimated_time = est_time,
        .priority = prio,
        .remaining_work = est_time,
        .arrival_time = (int)time(NULL),
        .deadline = deadline
    };

    if (from_left) {
        if (size_left >= MAX_THREADS) {
            fprintf(stderr, "[scheduler] Cola izquierda llena\n");
            CEmutex_unlock(&sched_lock);
            return;
        }
        ready_left[size_left++] = st;
    } else {
        if (size_right >= MAX_THREADS) {
            fprintf(stderr, "[scheduler] Cola derecha llena\n");
            CEmutex_unlock(&sched_lock);
            return;
        }
        ready_right[size_right++] = st;
    }

    CEmutex_unlock(&sched_lock);
    printf("[scheduler] Hilo agregado al lado %s. Total=%d\n", from_left ? "Izquierda" : "Derecha", from_left ? size_left : size_right);
}

static int select_index(ScheduledThread *queue, int size) {
    if (current_mode == SCHED_CE_FCFS || size == 0)
        return 0;

    int idx = 0;
    switch (current_mode) {
        case SCHED_CE_SJF: {
            int best = queue[0].estimated_time;
            for (int i = 1; i < size; ++i) {
                if (queue[i].estimated_time < best) {
                    best = queue[i].estimated_time;
                    idx = i;
                }
            }
        } break;

        case SCHED_CE_PRIORITY: {
            int best = queue[0].priority;
            for (int i = 1; i < size; ++i) {
                if (queue[i].priority < best) {
                    best = queue[i].priority;
                    idx = i;
                }
            }
        } break;

        case SCHED_CE_REALTIME: {
            int best = queue[0].deadline;
            for (int i = 1; i < size; ++i) {
                if (queue[i].deadline < best) {
                    best = queue[i].deadline;
                    idx = i;
                }
            }
        } break;

        case SCHED_CE_RR: {     
            idx = 0;
            break;
        }

        default:
            idx = 0;
            break;
    }

    return idx;
}

CEthread_t scheduler_next_thread_from_left(void) {
    CEmutex_lock(&sched_lock);
    if (size_left == 0) {
        CEmutex_unlock(&sched_lock);
        fprintf(stderr, "[scheduler] Cola izquierda vacía\n");
        exit(1);
    }
    int idx = select_index(ready_left, size_left);
    CEthread_t next = ready_left[idx].thread;
    for (int i = idx; i < size_left - 1; ++i)
        ready_left[i] = ready_left[i + 1];
        printf("[scheduler] Siguiente hilo desde izquierda: ID=%d\n", ready_left[idx].thread.tid);
    size_left--;
    CEmutex_unlock(&sched_lock);
    return next;
}

CEthread_t scheduler_next_thread_from_right(void) {
    CEmutex_lock(&sched_lock);
    
    if (size_right == 0) {
        CEmutex_unlock(&sched_lock);
        fprintf(stderr, "[scheduler] Cola derecha vacía\n");
        exit(1);
    }
    int idx = select_index(ready_right, size_right);
    CEthread_t next = ready_right[idx].thread;
    for (int i = idx; i < size_right - 1; ++i)
        ready_right[i] = ready_right[i + 1];
        printf("[scheduler] Siguiente hilo desde derecha: ID=%d\n", ready_right[idx].thread.tid);

    size_right--;
    CEmutex_unlock(&sched_lock);
    return next;
}

int scheduler_has_threads_left(void) {
    CEmutex_lock(&sched_lock);
    int result = (size_left > 0);
    CEmutex_unlock(&sched_lock);
    return result;
}

int scheduler_has_threads_right(void) {
    CEmutex_lock(&sched_lock);
    int result = (size_right > 0);
    CEmutex_unlock(&sched_lock);
    return result;
}


int scheduler_has_threads(void) {
    CEmutex_lock(&sched_lock);
    int alive = (size_left + size_right > 0);
    CEmutex_unlock(&sched_lock);
    return alive;
}

void scheduler_rr_report(pid_t tid, int unidades) {
    // Si más adelante querés usar RR por lado, implementalo aquí
    (void)tid;
    (void)unidades;
}

void scheduler_debug_print_right_queue(void) {
    CEmutex_lock(&sched_lock);
    printf("[DEBUG] Contenido de la cola derecha (%d hilos):\n", size_right);
    for (int i = 0; i < size_right; ++i) {
        printf("  -> Hilo ID=%d | TID=%d | est_time=%d\n",
               ready_right[i].thread.tid,
               ready_right[i].thread.tid,
               ready_right[i].estimated_time);
    }
    CEmutex_unlock(&sched_lock);
}

void scheduler_debug_print_queue(const char* lado) {
    CEmutex_lock(&sched_lock);

    ScheduledThread* queue;
    int size;

    if (strcmp(lado, "izquierda") == 0) {
        queue = ready_left;
        size = size_left;
    } else {
        queue = ready_right;
        size = size_right;
    }

    printf("[DEBUG] Cola %s (%d hilos):\n", lado, size);
    for (int i = 0; i < size; ++i) {
        printf("  -> ID=%d | TID=%d | est_time=%d | remaining=%d\n",
            queue[i].thread.tid,
            queue[i].thread.tid,
            queue[i].estimated_time,
            queue[i].remaining_work);
    }

    CEmutex_unlock(&sched_lock);
}

