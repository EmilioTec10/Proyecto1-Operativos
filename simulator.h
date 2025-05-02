#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "CEthreads.h"
#include "scheduler.h"

/* ── Lados de la carretera ───────────────────────────────── */
#define SIDE_LEFT    0
#define SIDE_RIGHT   1

/* ── Tipos de coche ──────────────────────────────────────── */
#define CAR_NORMAL    0
#define CAR_SPORT     1
#define CAR_EMERGENCY 2

#ifdef __cplusplus
extern "C" {
#endif

void simulator_init(CE_scheduler_mode_t mode,
                    int street_len,
                    int speed_base,
                    int sign_interval,
                    int W);

int simulator_add_car(int side, int type);

void simulator_start(void);

void simulator_cleanup(void);

#ifdef __cplusplus
}
#endif

#endif // SIMULATOR_H
