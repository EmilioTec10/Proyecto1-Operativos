/*  scheduler.c  — versión con exclusión mutua               */
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* ---------------- datos internos ------------------------- */
#define MAX_THREADS 100

static ScheduledThread ready_queue[MAX_THREADS];
static int             queue_size       = 0;
static CE_scheduler_mode_t current_mode;

/* ► nuevo cerrojo para proteger la cola */
static CEmutex_t sched_lock;

/* -------- helpers privados --------------------------------------- */
static void shift_left(int from)
{
    for (int k = from; k < queue_size - 1; ++k)
        ready_queue[k] = ready_queue[k + 1];
    queue_size--;
}
/* ----------------------------------------------------------------- */

void scheduler_init(CE_scheduler_mode_t mode)
{
    current_mode = mode;
    queue_size   = 0;

    CEmutex_init(&sched_lock);              /*  NEW  */

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

void scheduler_add_thread(CEthread_t th,
                          int est_time,
                          int prio,
                          int deadline)
{
    CEmutex_lock(&sched_lock);              /*  NEW  */

    if (queue_size >= MAX_THREADS) {
        CEmutex_unlock(&sched_lock);
        fprintf(stderr,"[scheduler] Cola llena\n");
        return;
    }

    ScheduledThread *st = &ready_queue[queue_size++];
    st->thread         = th;
    st->estimated_time = est_time;
    st->priority       = prio;
    st->arrival_time   = (int)time(NULL);
    st->remaining_work = est_time;
    st->deadline       = deadline;

    CEmutex_unlock(&sched_lock);            /*  NEW  */
    printf("[scheduler] Hilo agregado. Total=%d\n", queue_size);
}

CEthread_t scheduler_next_thread(void)
{
    CEmutex_lock(&sched_lock);              /*  NEW  */

    if (queue_size == 0) {
        CEmutex_unlock(&sched_lock);
        fprintf(stderr,"[scheduler] Cola vacía\n");
        exit(1);
    }

    /* --- limpiar terminados (sólo RR) --------------------- */
    if (current_mode == SCHED_CE_RR) {
        int i = 0;
        while (i < queue_size) {
            if (ready_queue[i].remaining_work <= 0)
                shift_left(i);              /* shift_left ya reduce queue_size */
            else
                i++;
        }
        if (queue_size == 0) {
            CEmutex_unlock(&sched_lock);
            fprintf(stderr,"[scheduler] Cola vacía después de limpiar\n");
            exit(1);
        }
    }

    /* --- elegir índice según política --------------------- */
    int idx = 0;
    switch (current_mode) {
        case SCHED_CE_FCFS:  idx = 0; break;

        case SCHED_CE_SJF: {
            int best = ready_queue[0].estimated_time;
            for (int i = 1; i < queue_size; ++i)
                if (ready_queue[i].estimated_time < best) {
                    best = ready_queue[i].estimated_time;
                    idx  = i;
                }
        } break;

        case SCHED_CE_PRIORITY: {
            int best = ready_queue[0].priority;
            for (int i = 1; i < queue_size; ++i)
                if (ready_queue[i].priority < best) {
                    best = ready_queue[i].priority;
                    idx  = i;
                }
        } break;

        case SCHED_CE_REALTIME: {
            int best = ready_queue[0].deadline;
            for (int i = 1; i < queue_size; ++i)
                if (ready_queue[i].deadline < best) {
                    best = ready_queue[i].deadline;
                    idx  = i;
                }
        } break;

        case SCHED_CE_RR:   idx = 0; break;

        default:
            CEmutex_unlock(&sched_lock);
            fprintf(stderr,"[scheduler] Modo desconocido\n");
            exit(1);
    }

    /* --- cola circular en RR / pop en otros modos ---------- */
    CEthread_t chosen_tid = ready_queue[idx].thread;

    if (current_mode == SCHED_CE_RR) {
        ScheduledThread temp = ready_queue[idx];
        for (int i = idx; i < queue_size - 1; ++i)
            ready_queue[i] = ready_queue[i + 1];
        ready_queue[queue_size - 1] = temp;
    } else {
        shift_left(idx);
    }

    CEmutex_unlock(&sched_lock);            /*  NEW  */
    return chosen_tid;
}

int scheduler_has_threads(void)
{
    CEmutex_lock(&sched_lock);              /*  NEW  */

    int alive = 0;

    if (current_mode == SCHED_CE_RR) {
        for (int i = 0; i < queue_size; ++i)
            if (ready_queue[i].remaining_work > 0) { alive = 1; break; }
    } else {
        alive = (queue_size > 0);
    }

    CEmutex_unlock(&sched_lock);            /*  NEW  */
    return alive;
}

/* -------- actualización desde los hijos RR ------------------------ */
void scheduler_rr_report(pid_t tid, int unidades)
{
    if (current_mode != SCHED_CE_RR) return;

    CEmutex_lock(&sched_lock);              /*  NEW  */

    for (int i = 0; i < queue_size; ++i)
        if (ready_queue[i].thread.tid == tid) {
            ready_queue[i].remaining_work -= unidades;
            break;
        }

    CEmutex_unlock(&sched_lock);            /*  NEW  */
}
