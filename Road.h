#ifndef ROAD_H
#define ROAD_H

#include "CEthreads.h"

// Definition of road sides
typedef enum { LEFT_SIDE, RIGHT_SIDE } Side;

// Structure representing the state of the road
typedef struct {
    int W;                        // Maximum number of cars that can cross from one side
    int counter;                  // Number of cars that have crossed from one side
    int cars_crossing;            // Number of cars currently crossing
    Side current_direction;       // The currently allowed side for crossing
    int waiting[2];               // Number of cars waiting on each side
    int cars_crossing_side;       // Side of the cars that are crossing
    CEmutex_t lock;               // Mutex to protect the state of the road
    CEmutex_t cond_var;           // Mutex for conditional waiting
} Road;

// Functions related to the road

// Initializes the road
void road_init(Road *r, int W);

// Frees the resources of the road
void road_destroy(Road *r);

// Blocking function. The thread waits until it can cross the road
void road_wait_for_turn(Road *r, Side side);

// Called when the thread finishes crossing. Frees space and updates the direction
void road_exit(Road *r, Side side);

// Function to manage the crossing of cars
void road_cross(Road *r, Side side);

#endif // ROAD_H
