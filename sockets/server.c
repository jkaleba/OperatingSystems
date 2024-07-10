#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#define MAX_CLIENTS 100
#define BUFFER_SIZE 1024
#define ID_SIZE 32

typedef struct {
    int sockfd;
    struct sockaddr_in address;
    int addr_len;
    char id[ID_SIZE];
    time_t last_active;
} client_t;

client_t clients[MAX_CLIENTS];
int num_clients = 0;
int server_socket;

void add_client(int new_sockfd, struct sockaddr_in *client_addr, int addr_len, const char *id) {
    if (num_clients < MAX_CLIENTS) {
        clients[num_clients].sockfd = new_sockfd;
        clients[num_clients].address = *client_addr;
        clients[num_clients].addr_len = addr_len;
        strncpy(clients[num_clients].id, id, ID_SIZE - 1);
        clients[num_clients].id[ID_SIZE - 1] = '\0';
        clients[num_clients].last_active = time(NULL);
        printf("New client connected: %s\n", clients[num_clients].id);
        num_clients++;
    } else {
        printf("Max clients reached. Cannot add more clients.\n");
    }
}

void remove_client(int client_index) {
    if (client_index < num_clients && client_index >= 0) {
        close(clients[client_index].sockfd);
        for (int i = client_index; i < num_clients - 1; i++) {
            clients[i] = clients[i + 1];
        }
        num_clients--;
    } else {
        printf("Invalid client index.\n");
    }
}

void broadcast_message(char *message, const char *sender_id) {
    char full_message[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(full_message, sizeof(full_message), "%s [%02d:%02d:%02d]: %s", sender_id, t->tm_hour, t->tm_min, t->tm_sec, message);

    for (int i = 0; i < num_clients; i++) {
        send(clients[i].sockfd, full_message, strlen(full_message), 0);
    }
}

void send_to_one(char *message, const char *sender_id, const char *recipient_id) {
    char full_message[BUFFER_SIZE];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    snprintf(full_message, sizeof(full_message), "%s [%02d:%02d:%02d]: %s", sender_id, t->tm_hour, t->tm_min, t->tm_sec, message);

    for (int i = 0; i < num_clients; i++) {
        if (strcmp(clients[i].id, recipient_id) == 0) {
            send(clients[i].sockfd, full_message, strlen(full_message), 0);
            break;
        }
    }
}

void list_clients(int sockfd) {
    char list[BUFFER_SIZE] = "Active clients:\n";
    for (int i = 0; i < num_clients; i++) {
        strcat(list, clients[i].id);
        strcat(list, "\n");
    }
    send(sockfd, list, strlen(list), 0);
}


void handle_client_message(int client_index, char *message) {

    clients[client_index].last_active = time(NULL);

    char *command = strtok(message, " ");
    if (strcmp(command, "LIST") == 0) {
        list_clients(clients[client_index].sockfd);
    }
    else if (strcmp(command, "2ALL") == 0) {
        char *msg = strtok(NULL, "");
        if (msg != NULL) {
            broadcast_message(msg, clients[client_index].id);
        }
    }
    else if (strcmp(command, "2ONE") == 0) {
        char *recipient_id = strtok(NULL, " ");
        char *msg = strtok(NULL, "");
        if (recipient_id != NULL && msg != NULL) {
            send_to_one(msg, clients[client_index].id, recipient_id);
        }
    }
    else if (strcmp(command, "STOP") == 0) {
        remove_client(client_index);
    }
}

void handle_client_communications() {
    fd_set readfds;
    int max_sd, activity;
    char buffer[BUFFER_SIZE];

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(server_socket, &readfds);
        max_sd = server_socket;

        for (int i = 0; i < num_clients; i++) {
            int sd = clients[i].sockfd;
            if (sd > 0) FD_SET(sd, &readfds);
            if (sd > max_sd) max_sd = sd;
        }

        select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_socket, &readfds)) {
            int new_socket;
            struct sockaddr_in address;
            int addrlen = sizeof(address);
            char id[ID_SIZE];

            if ((new_socket = accept(server_socket, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            int id_len = recv(new_socket, id, ID_SIZE - 1, 0);
            if (id_len < 0) {
                perror("recv");
                close(new_socket);
                continue;
            }

            id[id_len] = '\0';  // Ensure the string is null-terminated
            add_client(new_socket, &address, addrlen, id);
        }


        for (int i = 0; i < num_clients; i++) {
            int sd = clients[i].sockfd;
            if (FD_ISSET(sd, &readfds)) {
                int valread = read(sd, buffer, BUFFER_SIZE);
                if (valread == 0) {
                    remove_client(i);
                } else {
                    buffer[valread] = '\0';
                    handle_client_message(i, buffer);
                }
            }
        }
    }
}

void signal_handler(int signum) {
    for (int i = 0; i < num_clients; i++) {
        close(clients[i].sockfd);
    }
    close(server_socket);
    printf("Server shut down.\n");
    exit(signum);
}


pthread_mutex_t clients_mutex = PTHREAD_MUTEX_INITIALIZER;

void ping_clients() {
    char ping[] = "PING";
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        send(clients[i].sockfd, ping, strlen(ping), 0);
    }
    pthread_mutex_unlock(&clients_mutex);
}

void remove_inactive_clients() {
    time_t now = time(NULL);
    pthread_mutex_lock(&clients_mutex);
    for (int i = 0; i < num_clients; i++) {
        if (difftime(now, clients[i].last_active) > 30) {
            printf("Client %s timed out.\n", clients[i].id);
            remove_client(i);
            i--;
        }
    }
    pthread_mutex_unlock(&clients_mutex);
}

_Noreturn void * alive() {

    sleep(20);

    while (1) {
        printf("Pinging clients...\n");
        ping_clients();
        sleep(30);
        printf("Removing inactive clients...\n");
        remove_inactive_clients();
        sleep(10);
    }
}


int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    struct sockaddr_in server_addr;

    signal(SIGINT, signal_handler);

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 3) < 0) {
        perror("listen");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    pthread_t alive_thread;
    if (pthread_create(&alive_thread, NULL, alive, NULL) != 0) {
        perror("Could not create alive thread");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", port);

    handle_client_communications();


    return 0;

}
