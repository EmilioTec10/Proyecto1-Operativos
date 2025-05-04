cmake_minimum_required(VERSION 3.30)
project(Proyecto1_Operativos C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(Proyecto1_Operativos
    CEthreads.c
    CEthreads.h
    main.c
    scheduler.c
    scheduler.h
    planner.c
    planner.h)
