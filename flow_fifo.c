#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "flow_fifo.h"
#include "CEthreads.h"   // para getpid() y CEmutex

// Nodo de la cola FIFO
typedef struct Node {
    pid_t        tid;        // ID del hilo
    int          from_left;  // lado de llegada
    struct Node *next;
} Node;

static Node       *head = NULL, *tail = NULL;
static int         crossing = 0;     // 1 si hay carro cruzando
static CEmutex_t   lock;             // protege head/tail/crossing

void fifo_init(void) {
    head = tail = NULL;
    crossing = 0;
    CEmutex_init(&lock);
}

void fifo_request_pass(int from_left) {
    pid_t tid = getpid();
    // Crear nodo y encolarlo
    Node *n = malloc(sizeof(*n));
    if (!n) {
        perror("fifo_request_pass: malloc");
        exit(1);
    }
    n->tid       = tid;
    n->from_left = from_left;
    n->next      = NULL;

    CEmutex_lock(&lock);
    if (tail) tail->next = n;
    else      head       = n;
    tail = n;
    CEmutex_unlock(&lock);

    // Esperar turno: ser cabeza de la cola y que no haya nadie cruzando
    while (1) {
        CEmutex_lock(&lock);
        if (head && head->tid == tid && crossing == 0) {
            // Es nuestro turno
            crossing = 1;
            // Sacar de la cola
            Node *tmp = head;
            head = head->next;
            if (!head) tail = NULL;
            CEmutex_unlock(&lock);
            free(tmp);

            printf("✅ [TID %d] ENTRA a cruzar (FIFO) desde %s\n",
                   tid, from_left ? "Izquierda" : "Derecha");
            return;
        }
        CEmutex_unlock(&lock);
        usleep(1000);
    }
}

void fifo_leave(void) {
    CEmutex_lock(&lock);
    crossing = 0;
    CEmutex_unlock(&lock);
}
