#ifndef PLANNER_H
#define PLANNER_H

#include "CEthreads.h"
#include "scheduler.h"

typedef struct {
    int work;       /* total de unidades */
    int id;         /* solo para imprimir */
    int priority;   /* usado en PRIORITY  */
    int deadline;   /* usado en REALTIME  */
} CE_Job;

/* Lanza todos los hilos, configura el scheduler y espera a que terminen.
   - jobs[] : arreglo de descripciones
   - n      : cuántos elementos en jobs[]
   - mode   : algoritmo (FCFS / SJF / … / RR)
   - q      : quantum para RR (ignorado en otros modos) */
void ce_run_plan(const CE_Job jobs[], int n,
                 CE_scheduler_mode_t mode, int q);

#endif
