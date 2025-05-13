#ifndef FLOW_FIFO_H
#define FLOW_FIFO_H

#include <sys/types.h>

/**
 * Inicializa la estructura interna de FIFO.
 * Debe llamarse antes de crear los hilos.
 */
void fifo_init(void);

/**
 * Solicita permiso para cruzar.
 * — from_left: 1 si viene de la izquierda, 0 si de la derecha.
 * Bloquea al hilo hasta que sea el siguiente en la cola y no haya nadie cruzando.
 */
void fifo_request_pass(int from_left);

/**
 * Señala que terminó de cruzar, permitiendo que el siguiente en cola pase.
 */
void fifo_leave(void);

#endif // FLOW_FIFO_H
