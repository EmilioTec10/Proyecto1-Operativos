// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "CEthreads.h"  // Para poder usar CEthread_t sin hacer un .h

typedef enum {
    SCHED_CE_FCFS,
    SCHED_CE_SJF,
    SCHED_CE_PRIORITY,
    SCHED_CE_RR,
    SCHED_CE_REALTIME
} CE_scheduler_mode_t;

// Estructura para representar cada hilo planificado
typedef struct {            /* igual que antes */
    CEthread_t thread;
    int estimated_time;
    int priority;
    int arrival_time;
    int remaining_work;
    int deadline;
} ScheduledThread;

/* ───── NUEVO: tamaño de quantum RR (en “unidades” de run_thread) ─── */
#define CE_RR_QUANTUM   1

void scheduler_init(CE_scheduler_mode_t mode);
void scheduler_add_thread(CEthread_t thread, int estimated_time, int priority, int deadline);
CEthread_t scheduler_next_thread();
int scheduler_has_threads();


#endif