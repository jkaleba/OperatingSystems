#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <pthread.h>
#include <signal.h>

#define BUFFER_SIZE 1024
#define ID_SIZE 32

int sockfd;
char client_id[ID_SIZE];

void *receive_messages(void *arg) {
    char buffer[BUFFER_SIZE];
    int valread;
    while ((valread = read(sockfd, buffer, BUFFER_SIZE)) > 0) {
        buffer[valread] = '\0';
        printf("%s\n", buffer);
    }
    if (valread == 0) {
        printf("Server disconnected.\n");
    } else {
        perror("recv");
    }
    close(sockfd);
    exit(0);
}

void signal_handler(int signum) {
    send(sockfd, "STOP", strlen("STOP"), 0);
    close(sockfd);
    printf("Client shut down.\n");
    exit(signum);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <client_id> <server_ip> <server_port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // Ensure the client ID is properly null-terminated
    strncpy(client_id, argv[1], sizeof(client_id) - 1);
    client_id[sizeof(client_id) - 1] = '\0';  // Ensure the string is null-terminated

    char *server_ip = argv[2];
    int server_port = atoi(argv[3]);

    struct sockaddr_in serv_addr;

    signal(SIGINT, signal_handler);

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Socket creation error");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(server_port);

    if (inet_pton(AF_INET, server_ip, &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection failed");
        return -1;
    }

    // Send the properly null-terminated client ID to the server
    send(sockfd, client_id, strlen(client_id), 0);

    pthread_t recv_thread;
    if (pthread_create(&recv_thread, NULL, receive_messages, NULL) != 0) {
        perror("Could not create receive thread");
        return -1;
    }

    char buffer[BUFFER_SIZE];
    while (1) {
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline character
        send(sockfd, buffer, strlen(buffer), 0);
    }

    close(sockfd);
    return 0;
}
