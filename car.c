#include "car.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#define MAX_CARS 10

Car cars[MAX_CARS];
int num_cars = 0;

void* move_car(void* arg) {
    int id = *(int*)arg;
    while (1) {
        usleep(100000); // 100 ms
    }
    return NULL;
}

int create_cars(){
    int orden = 0, type = 0;
    for (int i = 0; i < 10; ++i) {
        if (i < 5){
            cars[i].direction = true;
        }else{
            cars[i].direction = false;
        }
        cars[i].orden = orden;
        cars[i].type = type;
        type++;
        orden++;
        if (type == 3){
            type = 0;
        }
        pthread_t tid;
        int* arg = malloc(sizeof(int));
        *arg = i;
        //pthread_create(&tid, NULL, move_car, arg);
    }
    num_cars = 10;

}