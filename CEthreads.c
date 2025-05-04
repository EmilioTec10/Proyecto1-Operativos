#define _GNU_SOURCE
#include <sched.h>
#include <sys/types.h>
#include <linux/sched.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdatomic.h>
#include <linux/futex.h>
#include <sys/syscall.h>
#include <stdint.h>

#define STACK_SIZE (1024 * 1024)

typedef struct {
    pid_t tid;
} CEthread_t;

typedef struct {
    atomic_int value;
} CEmutex_t;

typedef int (*CEthread_start_routine)(void *);

struct thread_args {
    CEthread_start_routine func;
    void *arg;
};

int thread_entry(void *void_args) {
    struct thread_args *args = (struct thread_args *)void_args;
    int result = args->func(args->arg);
    free(args);
    return result;
}

int CEthread_create(CEthread_t *thread, void *(*start_routine)(void *), void *arg) {
    void *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    void *stack_top = (char *)stack + STACK_SIZE;

    struct thread_args *args = malloc(sizeof(struct thread_args));
    if (!args) {
        munmap(stack, STACK_SIZE);
        return -1;
    }
    args->func = (CEthread_start_routine)start_routine;
    args->arg = arg;

    int flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | SIGCHLD;

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

int CEthread_join(CEthread_t thread) {
    int status;
    if (waitpid(thread.tid, &status, 0) == -1) {
        if (errno == ECHILD) return 0;
        perror("waitpid");
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

int CEmutex_init(CEmutex_t *mutex) {
    atomic_store(&mutex->value, 0);
    return 0;
}

int CEmutex_destroy(CEmutex_t *mutex) {
    if (atomic_load(&mutex->value) != 0) {
        return -1;
    }
    atomic_store(&mutex->value, -1);
    return 0;
}

int CEmutex_lock(CEmutex_t *mutex) {
    int expected;

    while (1) {
        expected = 0;
        if (atomic_compare_exchange_strong(&mutex->value, &expected, 1)) {
            return 0;
        }

        syscall(SYS_futex, &mutex->value, FUTEX_WAIT, 1, NULL, NULL, 0);
    }
}

int CEmutex_unlock(CEmutex_t *mutex) {
    atomic_store(&mutex->value, 0);
    syscall(SYS_futex, &mutex->value, FUTEX_WAKE, 1, NULL, NULL, 0);
    return 0;
}