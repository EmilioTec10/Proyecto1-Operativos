#include "server.h"
#include "car.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8000

void start_server() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    bind(server_fd, (struct sockaddr*)&address, sizeof(address));
    listen(server_fd, 3);

    printf("Esperando cliente...\n");
    client_fd = accept(server_fd, (struct sockaddr*)&address, (socklen_t*)&addrlen);
    printf("Cliente conectado.\n");

    while (1) {
        char buffer[1024] = {0};
        int offset = 0;

        for (int i = 0; i < num_cars; i++) {
            offset += snprintf(buffer + offset, sizeof(buffer) - offset,
                "%d,%d,%d,%d,%d,%d;",
                cars[i].x,
                cars[i].y,
                cars[i].speed,
                cars[i].moving,
                cars[i].direction,
                cars[i].priority
            );        }

        send(client_fd, buffer, strlen(buffer), 0);
        usleep(100000);
    }

    close(client_fd);
    close(server_fd);
}
