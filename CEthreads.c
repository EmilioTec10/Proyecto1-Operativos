#define _GNU_SOURCE
#include <sched.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define STACK_SIZE (1024 * 1024) // 1 MB

typedef struct {
    pid_t tid; // ID del hilo creado (thread ID)
} CEthread_t;

typedef int (*CEthread_start_routine)(void *);

// Wrapper para la función que correrá el hilo
struct thread_args {
    CEthread_start_routine func;
    void *arg;
};

// Función interna que se pasa a clone()
int thread_entry(void *void_args) {
    struct thread_args *args = (struct thread_args *)void_args;
    int result = args->func(args->arg);
    free(args); // liberamos memoria usada por argumentos
    return result;
}

int CEthread_create(CEthread_t *thread, void *(*start_routine)(void *), void *arg) {
    // Asignar pila con mmap
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    // Apuntar al tope de la pila (stack crece hacia abajo)
    void *stack_top = (char *)stack + STACK_SIZE;

    // Preparamos argumentos
    struct thread_args *args = malloc(sizeof(struct thread_args));
    if (!args) {
        munmap(stack, STACK_SIZE);
        return -1;
    }
    args->func = (CEthread_start_routine)start_routine;
    args->arg = arg;

    // Flags para simular un hilo (comparten memoria, archivos, señales)
    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD;

    // Crear hilo
    pid_t tid = clone(thread_entry, stack_top, flags, args);
    if (tid == -1) {
        perror("clone");
        free(args);
        munmap(stack, STACK_SIZE);
        return -1;
    }

    thread->tid = tid;
    return 0;
}
