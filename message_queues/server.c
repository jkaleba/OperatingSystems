#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <signal.h>
#include <unistd.h>

#define SERVER_QUEUE_NAME   "/server_queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

typedef struct client {
    mqd_t queue_descriptor;
    char queue_name[64];
} Client;

Client clients[MAX_MESSAGES];
int num_clients = 0;

void cleanup(int signum) {
    mq_unlink(SERVER_QUEUE_NAME);
    exit(0);
}

int main() {
    mqd_t qd_server, qd_client;
    char in_buffer[MSG_BUFFER_SIZE];
    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_server = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Server: mq_open (server)");
        exit(1);
    }

    signal(SIGINT, cleanup);

    printf("Server: Waiting for messages.\n");

    while (1) {
        if (mq_receive(qd_server, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
            perror("Server: mq_receive");
            continue;
        }

        char cmd[10], client_queue_name[64];
        sscanf(in_buffer, "%s %s", cmd, client_queue_name);

        if (strcmp(cmd, "INIT") == 0) {
            printf("Server: INIT received from %s\n", client_queue_name);

            Client new_client;
            sprintf(new_client.queue_name, "%s", client_queue_name);
            new_client.queue_descriptor = mq_open(client_queue_name, O_WRONLY);
            clients[num_clients++] = new_client;

            char msg[10];
            sprintf(msg, "%d", num_clients);
            mq_send(new_client.queue_descriptor, msg, strlen(msg) + 1, 0);
        }
        else {
            int client_id = atoi(cmd);
            printf("Server: Message from client %d: %s\n", client_id, client_queue_name);

            for (int i = 0; i < num_clients; i++) {
                if (i + 1 != client_id) {
                    mq_send(clients[i].queue_descriptor, client_queue_name, strlen(client_queue_name) + 1, 0);
                }
            }
        }
    }
}
