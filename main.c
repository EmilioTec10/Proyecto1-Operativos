#include <stdio.h>
#include "planner.h"

// Variables globales requeridas por planner/worker

int W = 3; // cantidad de carros que cruzan por turno en el algoritmo de equidad

int main() {
    CE_Job jobs[6];

    // Carros de la izquierda (IDs 0,1,2)
    for (int i = 0; i < 3; ++i) {
        jobs[i].id = i;
        jobs[i].work = 5;       // cada carro cruza en 5 pasos
        jobs[i].priority = 1;   // no importa para equidad
        jobs[i].deadline = 10;  // no importa para equidad
        jobs[i].from_left = 1;  // izquierda
    }

    // Carros de la derecha (IDs 3,4,5)
    for (int i = 3; i < 6; ++i) {
        jobs[i].id = i;
        jobs[i].work = 5;
        jobs[i].priority = 1;
        jobs[i].deadline = 10;
        jobs[i].from_left = 0;  // derecha
    }

    ce_run_plan(jobs, 6, SCHED_CE_FCFS, 0, W);  // modo de planificación FCFS
    return 0;
}
