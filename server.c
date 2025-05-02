// server.c (agregar al proyecto)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 65432
#define MAX_JSON_SIZE 1024

void send_state(int client_socket) {
    // Obtener el estado actual de las colas y carros activos
    char json_state[MAX_JSON_SIZE];
    snprintf(json_state, MAX_JSON_SIZE, 
        "{"
        "\"queues\": {\"left\": [], \"right\": []},"
        "\"active_cars\": [{\"position\": 50, \"lane\": 1, \"type\": \"normal\"}],"
        "\"current_sign\": \"left\""
        "}");
    send(client_socket, json_state, strlen(json_state), 0);
}

void handle_client(int client_socket) {
    char buffer[1024];
    while(1) {
        memset(buffer, 0, 1024);
        ssize_t bytes_read = recv(client_socket, buffer, 1024, 0);
        
        if(strncmp(buffer, "get_state", 9) == 0) {
            send_state(client_socket);
        }
        else if(strncmp(buffer, "exit", 4) == 0) {
            break;
        }
        // Implementar lógica para agregar carros
    }
    close(client_socket);
}

// server.c (extensión)
#include "cJSON.h"

char* generate_state_json() {
    cJSON *root = cJSON_CreateObject();
    
    // Colas
    cJSON *queues = cJSON_CreateObject();
    cJSON_AddItemToObject(queues, "left", cJSON_CreateArray());
    cJSON_AddItemToObject(queues, "right", cJSON_CreateArray());
    
    // Ejemplo: agregar carros en cola izquierda
    cJSON *car = cJSON_CreateObject();
    cJSON_AddStringToObject(car, "type", "normal");
    cJSON_AddItemToArray(cJSON_GetObjectItem(queues, "left"), car);
    
    cJSON_AddItemToObject(root, "queues", queues);
    
    // Carros activos
    cJSON *active_cars = cJSON_CreateArray();
    cJSON *moving_car = cJSON_CreateObject();
    cJSON_AddNumberToObject(moving_car, "position", 50);
    cJSON_AddStringToObject(moving_car, "type", "emergency");
    cJSON_AddItemToArray(active_cars, moving_car);
    
    cJSON_AddItemToObject(root, "active_cars", active_cars);
    cJSON_AddStringToObject(root, "current_sign", "left");
    
    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;
}

int main() {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in address = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    
    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 5);
    
    while(1) {
        int client_socket = accept(server_fd, NULL, NULL);
        handle_client(client_socket);
    }
    
    return 0;
}