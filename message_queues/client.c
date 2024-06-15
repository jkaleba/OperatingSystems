#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mqueue.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>

#define SERVER_QUEUE_NAME   "/server_queue"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE 256
#define MSG_BUFFER_SIZE MAX_MSG_SIZE + 10

void cleanup(int signum) {
    char client_queue_name[64];
    sprintf(client_queue_name, "/client_queue_%d", getpid());
    mq_unlink(client_queue_name);
    exit(0);
}

int main() {
    mqd_t qd_server, qd_client;
    char out_buffer[MSG_BUFFER_SIZE], in_buffer[MSG_BUFFER_SIZE];
    char client_queue_name[64];
    sprintf(client_queue_name, "/client_queue_%d", getpid());

    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_client = mq_open(client_queue_name, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS, &attr)) == -1) {
        perror("Client: mq_open (client)");
        exit(1);
    }

    if ((qd_server = mq_open(SERVER_QUEUE_NAME, O_WRONLY)) == -1) {
        perror("Client: mq_open (server)");
        exit(1);
    }

    signal(SIGINT, cleanup);

    char msg[MSG_BUFFER_SIZE];
    sprintf(msg, "INIT %s", client_queue_name);
    if (mq_send(qd_server, msg, strlen(msg) + 1, 0) == -1) {
        perror("Client: mq_send (INIT)");
        exit(1);
    }

    if (mq_receive(qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
        perror("Client: mq_receive");
        exit(1);
    }

    int client_id;
    sscanf(in_buffer, "%d", &client_id);
    printf("Client: Server assigned ID %d.\n", client_id);

    pid_t pid = fork();
    if (pid == 0) {
        while (1) {
            if (mq_receive(qd_client, in_buffer, MSG_BUFFER_SIZE, NULL) == -1) {
                perror("Client: mq_receive");
                continue;
            }
            printf("Received: %s\n", in_buffer);
        }
    } else {
        while (1) {
            printf("Enter message: ");
            fgets(out_buffer, MSG_BUFFER_SIZE, stdin);
            sprintf(msg, "%d %s", client_id, out_buffer);
            if (mq_send(qd_server, msg, strlen(msg) + 1, 0) == -1) {
                perror("Client: mq_send");
            }
        }
    }

    wait(NULL);
    cleanup(0);
}
