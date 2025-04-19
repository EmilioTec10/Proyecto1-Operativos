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
typedef struct {
    CEthread_t thread;
    int estimated_time;     // tiempo estimado (para SJF)
    int priority;           // prioridad (para PRIORITY)
    int arrival_time;       // tiempo de llegada (para FCFS)
    int remaining_work;     // trabajo pendiente (para RR)
    int deadline;           // tiempo real (para REALTIME)
} ScheduledThread;

void scheduler_init(CE_scheduler_mode_t mode);
void scheduler_add_thread(CEthread_t thread, int estimated_time, int priority, int deadline);
CEthread_t scheduler_next_thread();
int scheduler_has_threads();

#endif