#ifndef CETHREADS_H
#define CETHREADS_H

#include <sys/types.h>

typedef struct {
    pid_t tid;
} CEthread_t;

typedef struct {
    _Atomic int value;
} CEmutex_t;

// Firma de funciones disponibles
int CEthread_create(CEthread_t *thread, void *(*start_routine)(void *), void *arg);
int CEthread_join(CEthread_t thread);

int CEmutex_init(CEmutex_t *mutex);
int CEmutex_destroy(CEmutex_t *mutex);
int CEmutex_lock(CEmutex_t *mutex);
int CEmutex_unlock(CEmutex_t *mutex);

#endif
