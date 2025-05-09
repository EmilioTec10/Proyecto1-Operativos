#include <stdio.h>     
#include <unistd.h>    
#include "flow_equity.h"
#include "CEthreads.h"
#include "scheduler.h"

static int w; // cantidad de carros por turno
static int passed = 0;
static int current_dir = 0; // 0: izquierda->derecha, 1: derecha->izquierda
static int waiting_left = 0, waiting_right = 0;
static int crossing = 0;     // carros cruzando actualmente
static int active_tid = -1;  // TID del hilo que tiene permiso a cruzar


static CEmutex_t lock;
static CEmutex_t can_pass;

void equity_init(int w_param) {
    w = w_param;
    passed = 0;
    current_dir = 0;
    waiting_left = 0;
    waiting_right = 0;
    CEmutex_init(&lock);
    CEmutex_init(&can_pass);
}

void equity_request_pass(int from_left) {
    int my_dir = from_left ? 0 : 1;
    pid_t tid = getpid();

    CEmutex_lock(&lock);
    if (from_left)
        waiting_left++;
    else
        waiting_right++;
    CEmutex_unlock(&lock);

    while (1) {
        CEmutex_lock(&lock);

        int current_waiting = (current_dir == 0) ? waiting_left : waiting_right;
        int opposite_waiting = (current_dir == 0) ? waiting_right : waiting_left;

       
        if (crossing == 0) {
            // Cambio normal después de W pasos
            if (passed >= w && (opposite_waiting > 0 || current_waiting == 0)) {
                current_dir = 1 - current_dir;
                passed = 0;
                printf("🔁 Cambio de dirección (normal)\n");
            }
        
            // 🔥 Cambio forzado si no hay nadie más en el lado actual
            else if (current_waiting == 0 && opposite_waiting > 0) {
                current_dir = 1 - current_dir;
                passed = 0;
                printf("🔁 Cambio de dirección (forzado por lado vacío)\n");
            }
        }

        int can_pass = 0;

        if (current_dir == my_dir && passed < w) {
            can_pass = 1;
        }

        if (can_pass) {
            passed++;
            crossing++;
            if (from_left) waiting_left--;
            else waiting_right--;

            printf("✅ [TID %d] ENTRA a cruzar desde %s\n", tid,
                   from_left ? "Izquierda" : "Derecha");

            CEmutex_unlock(&lock);
            break;
        }

        CEmutex_unlock(&lock);
        usleep(1000);
    }
}

void equity_leave() {
    CEmutex_lock(&lock);
    crossing--;

    int current_waiting = (current_dir == 0) ? waiting_left : waiting_right;
    int opposite_waiting = (current_dir == 0) ? waiting_right : waiting_left;

    if (crossing == 0) {
        // condición 1: se alcanzó el W → cambio normal
        if (passed >= w) {
            if (opposite_waiting > 0 || current_waiting == 0) {
                current_dir = 1 - current_dir;
                passed = 0;
                printf("🔁 Cambiando dirección: ahora %s\n",
                       current_dir == 0 ? "Izquierda -> Derecha" : "Derecha -> Izquierda");
            }
        }
        // condición 2: no se llegó al W, pero ya no hay nadie del lado actual
        else if (current_waiting == 0 && opposite_waiting > 0) {
            current_dir = 1 - current_dir;
            passed = 0;
            printf("🔁 (Forzado) Cambiando dirección: ahora %s\n",
                   current_dir == 0 ? "Izquierda -> Derecha" : "Derecha -> Izquierda");
        }
    }

    CEmutex_unlock(&lock);
}



