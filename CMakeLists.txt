cmake_minimum_required(VERSION 3.10)
project(Proyecto1_Operativos C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

# → Librería compartida con toda tu lógica de CEthreads + scheduler + simulador
add_library(cethreads SHARED
    CEthreads.c
    scheduler.c
    simulator.c        # (ver más abajo)
)

# → Ejecutable de pruebas (opcional)
add_executable(Proyecto1_Operativos
    main.c
)

target_link_libraries(Proyecto1_Operativos cethreads)