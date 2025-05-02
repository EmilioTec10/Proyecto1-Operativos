#include "simulator.h"
#include <unistd.h>    // usleep()
#include <time.h>      // time()
#include <stdio.h>
#include <stdlib.h>

/* ── Contexto interno para cada coche ─────────────────────── */
typedef struct {
    int work, id, priority, deadline;
    int side;
    CEmutex_t start_lock;
    CEthread_t tid;
} CarContext;

/* ── Estado estático del simulador ───────────────────────── */
static CarContext           *cars       = NULL;
static int                   car_count  = 0;
static int                   max_cars   = 0;
static int                   street_len;
static int                   speed_base;
static int                   sign_interval;
static int                   W;
static CE_scheduler_mode_t   sim_mode;

/* ── Nuestro “counter” y su mutex ─────────────────────────── */
static int       sim_counter = 0;
static CEmutex_t sim_lock;

/* ── Función que corre cada hilo “coche” ──────────────────── */
static void *car_thread_fn(void *arg) {
    CarContext *c = (CarContext *)arg;

    // barrera de arranque
    CEmutex_lock(&c->start_lock);
    CEmutex_unlock(&c->start_lock);

    printf("[Car %2d] side=%s type=%d work=%d prio=%d dl=%d\n",
           c->id,
           c->side == SIDE_LEFT ? "LEFT" : "RIGHT",
           c->priority,
           c->work,
           c->deadline);

    for (int i = 0; i < c->work; ++i) {
        // sección crítica: incrementamos nuestro counter
        CEmutex_lock(&sim_lock);
        sim_counter++;
        CEmutex_unlock(&sim_lock);

        // simulamos un pequeño retardo según prioridad
        usleep(1000 * (100 - c->priority));
    }

    printf("[Car %2d] completed\n", c->id);
    return NULL;
}

void simulator_init(CE_scheduler_mode_t mode,
                    int _street_len,
                    int _speed_base,
                    int _sign_interval,
                    int _W)
{
    sim_mode      = mode;
    street_len    = _street_len;
    speed_base    = _speed_base;
    sign_interval = _sign_interval;
    W             = _W;

    max_cars   = 100;
    cars       = calloc(max_cars, sizeof(CarContext));
    car_count  = 0;

    // Inicializamos nuestro counter interno y su mutex
    sim_counter = 0;
    CEmutex_init(&sim_lock);

    // Inicializamos el scheduler
    scheduler_init(sim_mode);
}

int simulator_add_car(int side, int type) {
    if (car_count >= max_cars) return -1;
    CarContext *c = &cars[car_count];

    c->id       = car_count;
    c->side     = side;

    // calculamos work, prio y deadline según tipo
    switch (type) {
      case CAR_NORMAL:
        c->work     = street_len / speed_base;
        c->priority = 1;
        c->deadline = 0;
        break;
      case CAR_SPORT:
        c->work     = street_len / (speed_base + 1);
        c->priority = 2;
        c->deadline = 0;
        break;
      case CAR_EMERGENCY:
        c->work     = street_len / (speed_base + 2);
        c->priority = 5;
        c->deadline = sign_interval;
        break;
      default:
        return -1;
    }

    // barrera de arranque
    CEmutex_init(&c->start_lock);
    CEmutex_lock(&c->start_lock);

    // creamos el hilo y lo agregamos al scheduler
    CEthread_create(&c->tid, car_thread_fn, c);
    scheduler_add_thread(c->tid, c->work, c->priority, c->deadline);

    return car_count++;
}

void simulator_start(void) {
    while (scheduler_has_threads()) {
        CEthread_t next = scheduler_next_thread();

        // buscamos el contexto correspondiente
        CarContext *c = NULL;
        for (int i = 0; i < car_count; ++i) {
            if (cars[i].tid.tid == next.tid) {
                c = &cars[i];
                break;
            }
        }

        // desbloqueamos la barrera para que corra
        CEmutex_unlock(&c->start_lock);
        CEthread_join(next);
    }

    printf("=== Simulation complete. sim_counter = %d ===\n",
           sim_counter);
}

void simulator_cleanup(void) {
    // destruimos nuestro mutex
    CEmutex_destroy(&sim_lock);

    // destruimos barreras individuales
    for (int i = 0; i < car_count; ++i) {
        CEmutex_destroy(&cars[i].start_lock);
    }
    free(cars);
    cars      = NULL;
    car_count = max_cars = 0;
}
