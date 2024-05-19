#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define PORT 8080
#define MAX_SIZE 1024

void handle_client(int new_socket) {
    char buffer[MAX_SIZE];
    FILE *file = fopen("index.html", "r");
    if (!file) {
        perror("Błąd odczytu pliku index.html");
        exit(EXIT_FAILURE);
    }
    snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n");
    send(new_socket, buffer, strlen(buffer), 0);
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, MAX_SIZE, file)) > 0) {
        send(new_socket, buffer, bytes_read, 0);
    }
    fclose(file);
    close(new_socket);
}

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;

    time_t current_time = time(NULL);
    char *times= ctime(&current_time);
    char *author = "Paweł Pieczykolan";

    printf("Serwer uruchomiony przez: %s\nData uruchomienia: %sSerwer nasłuchuje na porcie: %d\n", author, times, PORT);
    fflush(stdout);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Nie można utworzyć gniazda");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0 || listen(server_fd, 3) < 0) {
        perror("Błąd podczas bindowania lub nasłuchiwania");
        exit(EXIT_FAILURE);
    }

    while ((new_socket = accept(server_fd, NULL, NULL)) >= 0) {
        handle_client(new_socket);
    }

    perror("Błąd podczas akceptowania połączenia");
    return EXIT_FAILURE;
}
