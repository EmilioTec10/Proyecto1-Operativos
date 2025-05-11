#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <sys/stat.h>  // Para mkdir
#include <errno.h>     // Para errno

#define MAX_THREADS 100

static FILE* log_file = NULL; 

static ScheduledThread ready_left[MAX_THREADS];
static ScheduledThread ready_right[MAX_THREADS];
static int size_left = 0;
static int size_right = 0;

static CE_scheduler_mode_t current_mode;
static CEmutex_t sched_lock;

void scheduler_init(CE_scheduler_mode_t mode) {
    // Intentar crear el directorio logs, verificando errores
    int dir_result = mkdir("../logs", 0755);
    if (dir_result == -1 && errno != EEXIST) {
        perror("[scheduler] Error al crear directorio logs");
        exit(EXIT_FAILURE);
    }

    log_file = fopen("../logs/scheduler_logs.txt", "w");
    if (!log_file) {
        perror("[scheduler] Error al abrir logs/scheduler_logs.txt");
        exit(EXIT_FAILURE);
    }

    current_mode = mode;
    size_left = size_right = 0;
    CEmutex_init(&sched_lock);

    // Modificar todos los printf/fprintf existentes para usar log_file
    fprintf(log_file, "[scheduler] Algoritmo inicializado: ");
    switch (mode) {
        case SCHED_CE_FCFS:     fprintf(log_file, "FCFS\n");        break;
        case SCHED_CE_SJF:      fprintf(log_file, "SJF\n");         break;
        case SCHED_CE_PRIORITY: fprintf(log_file, "PRIORITY\n");    break;
        case SCHED_CE_REALTIME: fprintf(log_file, "REALTIME\n");    break;
        case SCHED_CE_RR:       fprintf(log_file, "Round Robin\n"); break;
        default:                fprintf(log_file, "Desconocido\n");
    }
    fflush(log_file);  // Forzar escritura al disco
}

void scheduler_cleanup() {
    if (log_file) {
        fprintf(log_file, "[scheduler] Cerrando log\n");
        fflush(log_file);  // Asegurar que todo se escribe antes de cerrar
        fclose(log_file);
        log_file = NULL;
    }
}

void scheduler_add_thread(CEthread_t th, int est_time, int prio, int deadline, int from_left) {
    CEmutex_lock(&sched_lock);

    fprintf(log_file, "[scheduler] Hilo ID=%d agregado al lado %s. Total=%d\n",
        th.tid, from_left ? "Izquierda" : "Derecha", from_left ? size_left : size_right);
    fflush(log_file);  // Forzar escritura

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
            fprintf(log_file, "[scheduler] Cola izquierda llena\n"); 
            fflush(log_file);  // Forzar escritura
            CEmutex_unlock(&sched_lock);
            return;
        }
        ready_left[size_left++] = st;
    } else {
        if (size_right >= MAX_THREADS) {
            fprintf(log_file, "[scheduler] Cola derecha llena\n");
            fflush(log_file);  // Forzar escritura
            CEmutex_unlock(&sched_lock);
            return;
        }
        ready_right[size_right++] = st;
    }

    fprintf(log_file, "[scheduler] Hilo agregado al lado %s. Total=%d\n", 
           from_left ? "Izquierda" : "Derecha", from_left ? size_left : size_right);
    fflush(log_file);  // Forzar escritura
    CEmutex_unlock(&sched_lock);
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

        case SCHED_CE_RR:
        default:
            idx = 0;
            break;
    }

    return idx;
}

CEthread_t scheduler_next_thread_from_left(void) {
    CEmutex_lock(&sched_lock);
    if (size_left == 0) {
        fprintf(log_file, "[scheduler] Cola izquierda vacía\n");
        fflush(log_file);  // Forzar escritura
        CEmutex_unlock(&sched_lock);  // Asegurar que se libera el mutex
        exit(1);
    }
    int idx = select_index(ready_left, size_left);
    CEthread_t next = ready_left[idx].thread;
    fprintf(log_file, "[scheduler] Siguiente hilo desde izquierda: ID=%d\n", next.tid);
    fflush(log_file);  // Forzar escritura
    
    for (int i = idx; i < size_left - 1; ++i)
        ready_left[i] = ready_left[i + 1];
    size_left--;
    CEmutex_unlock(&sched_lock);
    return next;
}

CEthread_t scheduler_next_thread_from_right(void) {
    CEmutex_lock(&sched_lock);
    
    if (size_right == 0) {
        fprintf(log_file, "[scheduler] Cola derecha vacía\n");
        fflush(log_file);  // Forzar escritura
        CEmutex_unlock(&sched_lock);  // Asegurar que se libera el mutex
        exit(1);
    }
    int idx = select_index(ready_right, size_right);
    CEthread_t next = ready_right[idx].thread;
    
    // Corregir la indentación y ubicación de este fprintf
    fprintf(log_file, "[scheduler] Siguiente hilo desde derecha: ID=%d\n", next.tid);
    fflush(log_file);  // Forzar escritura
    
    for (int i = idx; i < size_right - 1; ++i)
        ready_right[i] = ready_right[i + 1];
    
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
    fprintf(log_file, "[DEBUG] Contenido de la cola derecha (%d hilos):\n", size_right);
    fflush(log_file);  // Forzar escritura
    
    for (int i = 0; i < size_right; ++i) {
        fprintf(log_file, "  -> Hilo ID=%d | TID=%d | est_time=%d\n",
               ready_right[i].thread.tid,
               ready_right[i].thread.tid,
               ready_right[i].estimated_time);
        fflush(log_file);  // Forzar escritura
    }
    CEmutex_unlock(&sched_lock);
}