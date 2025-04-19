// scheduler.h
#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "CEthreads.h"  // Para poder usar CEthread_t sin hacer un .h

typedef enum {
    SCHED_ALGO_FCFS,
    SCHED_ALGO_SJF,
    SCHED_ALGO_PRIORITY,
    SCHED_ALGO_RR,
    SCHED_ALGO_REALTIME
} scheduler_algo_t;


typedef struct {
    CEthread_t thread;
    int tiempo_estimado;
    int prioridad;
    int tiempo_llegada;
} ScheduledThread;

void scheduler_init(scheduler_algo_t algo);
void scheduler_add_thread(CEthread_t thread, int tiempo_estimado, int prioridad);
CEthread_t scheduler_next_thread();
int scheduler_has_threads();

#endif
