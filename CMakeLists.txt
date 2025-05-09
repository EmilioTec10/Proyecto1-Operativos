cmake_minimum_required(VERSION 3.16)
project(Proyecto1_Operativos C)

set(CMAKE_C_STANDARD 11)

include_directories(.)
find_package(Threads REQUIRED)

add_executable(Proyecto1_Operativos
    CEthreads.c
    CEthreads.h
    main.c
    scheduler.c
    scheduler.h
    planner.c
    planner.h
    flow_equity.c
    flow_equity.h
    car.c
    car.h
    server.c
    server.h)
target_link_libraries(Proyecto1_Operativos Threads::Threads)
