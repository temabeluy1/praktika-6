#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 1024

void interactive_mode(int socket) {
    char buffer[BUFFER_SIZE];
    int guess;

    while (1) {
        printf("Enter your guess (1-100): ");
        scanf("%d", &guess);
        snprintf(buffer, BUFFER_SIZE, "%d", guess);

        // Отправка числа на сервер
        send(socket, buffer, strlen(buffer) + 1, 0);

        // Получение ответа от сервера
        memset(buffer, 0, BUFFER_SIZE);
        recv(socket, buffer, BUFFER_SIZE, 0);

        if (strncmp(buffer, "less", 4) == 0) {
            printf("The target number is smaller.\n");
        } else if (strncmp(buffer, "greater", 7) == 0) {
            printf("The target number is greater.\n");
        } else if (strncmp(buffer, "correct", 7) == 0) {
            int attempts;
            sscanf(buffer, "correct:%d", &attempts);
            printf("Correct! You guessed the number in %d attempts.\n", attempts);
            break;
        } else {
            printf("Unexpected response: %s\n", buffer);
        }
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <server_address> <port>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int client_socket;
    struct sockaddr_in server_addr;

    // Создание сокета
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket < 0) {
        perror("Socket creation failed");
        return EXIT_FAILURE;
    }

    // Настройка адреса сервера
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[2]));

    if (inet_pton(AF_INET, argv[1], &server_addr.sin_addr) <= 0) {
        perror("Invalid address");
        close(client_socket);
        return EXIT_FAILURE;
    }

    // Подключение к серверу
    if (connect(client_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(client_socket);
        return EXIT_FAILURE;
    }

    printf("Connected to the server. Start guessing!\n");
    interactive_mode(client_socket);

    close(client_socket);
    return EXIT_SUCCESS;
}
