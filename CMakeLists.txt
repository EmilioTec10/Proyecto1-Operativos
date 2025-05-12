cmake_minimum_required(VERSION 3.16)
project(PROYECTO1-OPERATIVO C)

# Opciones de compilación
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# Incluir directorio de cabeceras
include_directories(include)

# Listar todos los archivos fuente
set(SOURCES
    src/CEthreads.c
    src/main.c
    src/flow_equity.c
    src/flow_fifo.c
    src/planner.c
    src/scheduler.c
)

# Crear el ejecutable principal
add_executable(proyecto1 ${SOURCES})

# Si necesitas enlazar con bibliotecas específicas, por ejemplo pthreads:
#find_package(Threads REQUIRED)
#target_link_libraries(proyecto1 Threads::Threads)

# Si necesitas más opciones como bibliotecas matemáticas:
# target_link_libraries(proyecto1 m)