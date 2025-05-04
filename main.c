#include <stdio.h>
#include "planner.h"

int main(void)
{
    CE_Job carga[] = {
        {66, 1, 2, 10},
        {99, 2, 5,  5},
        {200, 3, 5,  5},
        {100, 4, 5,  5}

    };
    int n = sizeof(carga)/sizeof(carga[0]);
    // run_test(SCHED_CE_FCFS);      // First-Come, First-Served
    // run_test(SCHED_CE_SJF);       // Shortest Job First
    // run_test(SCHED_CE_PRIORITY);  // Prioridad
    // run_test(SCHED_CE_RR);        // Round Robin
    // run_test(SCHED_CE_REALTIME);  // Tiempo Real (según deadline)

    ce_run_plan(carga, n, SCHED_CE_PRIORITY, /*quantum*/5);
    /* Para probar otro modo:                          *
     * ce_run_plan(carga, n, SCHED_CE_PRIORITY, 0);    */

    return 0;
}
