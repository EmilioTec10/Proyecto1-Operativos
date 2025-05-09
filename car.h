#ifndef CAR_H
#define CAR_H
#include <stdbool.h>
#include <pthread.h>

typedef struct {
    int x;
    int y;
    int speed;
    int priority;
    bool moving;
    bool direction;
} Car;

extern Car cars[];
extern int num_cars;

void* move_car(void* arg);
int create_cars();
#endif
