#ifndef CAR_H
#define CAR_H
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    int orden;
    int type;
    bool direction;
    int speed;
} Car;

extern Car cars[];
extern int num_cars;

void* move_car(void* arg);
int create_cars();
#endif