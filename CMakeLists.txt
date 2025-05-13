cmake_minimum_required(VERSION 3.16)
project(PROYECTO1_OPERATIVO C)

# Opciones de compilación
set(CMAKE_C_STANDARD 11)
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")

# Incluir el directorio de cabeceras
include_directories(include)

# Crear una biblioteca con todo el código excepto main.c
# Esto permite que los tests usen las mismas funciones que el programa principal
set(LIB_SOURCES
    src/CEthreads.c
    src/scheduler.c
    src/planner.c
    src/flow_equity.c
    src/flow_fifo.c
)
add_library(proyecto_lib STATIC ${LIB_SOURCES})
target_include_directories(proyecto_lib PUBLIC include)

# Crear el ejecutable principal usando la biblioteca
add_executable(proyecto1 src/main.c)
target_link_libraries(proyecto1 proyecto_lib)

# Habilitar testing y añadir la carpeta tests
enable_testing()
add_subdirectory(tests)