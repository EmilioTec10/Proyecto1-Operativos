#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "CEthreads.h"

typedef enum {
    SCHED_CE_FCFS,
    SCHED_CE_SJF,
    SCHED_CE_PRIORITY,
    SCHED_CE_RR,
    SCHED_CE_REALTIME
} CE_scheduler_mode_t;

typedef struct {
    CEthread_t thread;
    int estimated_time;
    int priority;
    int remaining_work;
    int deadline;
    int arrival_time;
} ScheduledThread;

#define CE_RR_QUANTUM  5

void  scheduler_init(CE_scheduler_mode_t mode);
void  scheduler_add_thread(CEthread_t thread,
                           int estimated_time,
                           int priority,
                           int deadline,
                           int from_left); // nuevo parámetro para doble cola
CEthread_t scheduler_next_thread_from_left(void);
CEthread_t scheduler_next_thread_from_right(void);
int scheduler_has_threads_left(void);
int scheduler_has_threads_right(void);
int   scheduler_has_threads(void);
void  scheduler_rr_report(pid_t tid,int unidades);
void scheduler_debug_print_right_queue(void);

void scheduler_cleanup(void);


#endif
