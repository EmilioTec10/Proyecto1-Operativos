#include "car.h"
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>

#define MAX_CARS 10

Car cars[MAX_CARS];
int num_cars = 0;

void* move_car(void* arg) {
    int id = *(int*)arg;
    while (1) {
        usleep(100000); // 100 ms
        cars[id].x += cars[id].speed;
    }
    return NULL;
}

int create_cars(){
    int x = 10, y = 150, priority = 0, speed = 10;
    for (int i = 0; i < 6; ++i) {
        if (i < 3){
            cars[i].direction = true;
        }else if (i == 3){
            cars[i].direction = false;
            x = 900, y = 150;
        }else{
            cars[i].direction = false;
        }
        cars[i].x = x;
        cars[i].y = y;
        cars[i].priority = priority;
        cars[i].speed = speed;
        priority++;
        speed+=10;
        y += 140;
        if (priority == 3){
            priority = 0;
            speed=10;
        }
        pthread_t tid;
        int* arg = malloc(sizeof(int));
        *arg = i;
        //pthread_create(&tid, NULL, move_car, arg);
    }
    num_cars = 6;

}