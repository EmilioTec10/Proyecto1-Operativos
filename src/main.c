#include <stdio.h>
#include "../include/planner.h"

// Globales requeridas (aunque no se usen con SJF)
int W = 2;

int main() {
    CE_Job jobs[6];

    // ID 0 - Izquierda - trabajo corto
    jobs[0] = (CE_Job){ .id = 0, .work = 2, .priority = 1, .deadline = 10, .from_left = 1 };

    // ID 1 - Derecha - trabajo largo
    jobs[1] = (CE_Job){ .id = 1, .work = 8, .priority = 1, .deadline = 10, .from_left = 0 };

    // ID 2 - Izquierda - medio
    jobs[2] = (CE_Job){ .id = 2, .work = 5, .priority = 1, .deadline = 10, .from_left = 1 };

    // ID 3 - Derecha - muy corto
    jobs[3] = (CE_Job){ .id = 3, .work = 1, .priority = 1, .deadline = 10, .from_left = 0 };

    // ID 4 - Izquierda - medio
    jobs[4] = (CE_Job){ .id = 4, .work = 4, .priority = 1, .deadline = 10, .from_left = 1 };

    // ID 5 - Derecha - medio
    jobs[5] = (CE_Job){ .id = 5, .work = 5, .priority = 1, .deadline = 10, .from_left = 0 };

    // Ejecución con algoritmo SJF (Shortest Job First)
    ce_run_plan(jobs, 6, SCHED_CE_FCFS, 0, W);

    // SCHED_CE_FCFS
    // SCHED_CE_SJF
    // SCHED_CE_PRIORITY
    // SCHED_CE_RR
    // SCHED_CE_REALTIME

    return 0;
}
