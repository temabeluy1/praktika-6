#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define MAX_CLIENTS 5

void handle_client(int client_socket, struct sockaddr_in client_addr) {
    char buffer[BUFFER_SIZE];
    int target_number = rand() % 100 + 1; // Загаданное число от 1 до 100
    int guess, attempts = 0;

    printf("Client connected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        // Получить сообщение от клиента
        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received <= 0) {
            printf("Client disconnected: %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            break;
        }

        // Парсинг числа
        guess = atoi(buffer);
        attempts++;

        printf("%s:%d - Received: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), guess);

        if (guess > target_number) {
            send(client_socket, "less", strlen("less") + 1, 0);
        } else if (guess < target_number) {
            send(client_socket, "greater", strlen("greater") + 1, 0);
        } else {
            snprintf(buffer, BUFFER_SIZE, "correct:%d", attempts);
            send(client_socket, buffer, strlen(buffer) + 1, 0);
            printf("%s:%d - Correct guess in %d attempts\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), attempts);
            break;
        }
    }

    close(client_socket);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len = sizeof(client_addr);
    int port = atoi(argv[1]);

    srand(time(NULL)); // Инициализация генератора случайных чисел

    // Создание сокета
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    // Привязка сокета
    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Bind failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    // Ожидание подключений
    if (listen(server_socket, MAX_CLIENTS) < 0) {
        perror("Listen failed");
        close(server_socket);
        return EXIT_FAILURE;
    }

    printf("Server listening on port %d...\n", port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr *)&client_addr, &client_len);
        if (client_socket < 0) {
            perror("Accept failed");
            continue;
        }

        if (!fork()) {
            close(server_socket);
            handle_client(client_socket, client_addr);
            exit(0);
        }

        close(client_socket);
    }

    close(server_socket);
    return EXIT_SUCCESS;
}
