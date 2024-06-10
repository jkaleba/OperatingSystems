#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <semaphore.h>
#include <time.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_PRINT_JOBS 10
#define NUM_PRINTERS 1
#define NUM_USERS 2
#define PRINT_JOB_SIZE 10

typedef struct {
    char text[PRINT_JOB_SIZE + 1];
    int busy;
} PrintJob;

sem_t *queue_mutex;
sem_t *empty_slots;
sem_t *filled_slots;
PrintJob *print_queue;

_Noreturn void printer_function(int printer_id) {
    while (1) {
        if (sem_wait(filled_slots) == -1) {
            perror("sem_wait filled_slots failed");
            continue;
        }
        if (sem_wait(queue_mutex) == -1) {
            perror("sem_wait queue_mutex failed");
            continue;
        }

        int job_found = 0;
        for (int i = 0; i < MAX_PRINT_JOBS; i++) {
            if (print_queue[i].busy) {
                printf("Printer %d starts printing: %s\n", printer_id, print_queue[i].text);
                fflush(stdout);
                print_queue[i].busy = 0;
                job_found = 1;

                sem_post(queue_mutex);

                for (int j = 0; j < PRINT_JOB_SIZE; j++) {
                    printf("%c", print_queue[i].text[j]);
                    fflush(stdout);
                    nanosleep((struct timespec[]){{0, 500000000}}, NULL);
                }
                printf("\n");
            }
        }

        if (job_found) {
            sem_post(empty_slots);
        }
    }
}

_Noreturn void user_function(int user_id) {
    srand(time(NULL) + getpid() + user_id);

    while (1) {

        if (sem_wait(empty_slots) == -1) {
            perror("sem_wait empty_slots failed");
            continue;
        }
        if (sem_wait(queue_mutex) == -1) {
            perror("sem_wait queue_mutex failed");
            continue;
        }

        char job_text[PRINT_JOB_SIZE + 1];
        for (int i = 0; i < PRINT_JOB_SIZE; i++) {
            job_text[i] = 'a' + rand() % 26;
        }
        job_text[PRINT_JOB_SIZE] = '\0';

        printf("User %d generated a print job: %s\n", user_id, job_text);
        fflush(stdout);


        int enqueued = 0;
        for (int i = 0; i < MAX_PRINT_JOBS; i++) {
            if (!print_queue[i].busy) {
                strcpy(print_queue[i].text, job_text);
                print_queue[i].busy = 1;
                printf("User %d enqueued a job: %s\n", user_id, job_text);
                fflush(stdout);
                enqueued = 1;
                break;
            }
        }

        sem_post(queue_mutex);
        if (enqueued) {
            sem_post(filled_slots);
        }

        sleep(10 + rand() % 10);
    }
}

int main() {
    sem_unlink("/queue_mutex");
    sem_unlink("/empty_slots");
    sem_unlink("/filled_slots");

    queue_mutex = sem_open("/queue_mutex", O_CREAT, 0644, 1);
    empty_slots = sem_open("/empty_slots", O_CREAT, 0644, MAX_PRINT_JOBS);
    filled_slots = sem_open("/filled_slots", O_CREAT, 0644, 0);

    int shm_fd = shm_open("/print_queue", O_CREAT | O_RDWR, 0666);
    ftruncate(shm_fd, sizeof(PrintJob) * MAX_PRINT_JOBS);
    print_queue = mmap(0, sizeof(PrintJob) * MAX_PRINT_JOBS, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

    for (int i = 0; i < MAX_PRINT_JOBS; i++) {
        print_queue[i].busy = 0;
    }

    for (int i = 0; i < NUM_PRINTERS + NUM_USERS; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            if (i < NUM_USERS) {
                user_function(i);
            } else {
                printer_function(i - NUM_USERS);
            }
            exit(0);
        }
    }

    while (wait(NULL) > 0);

    sem_close(queue_mutex);
    sem_close(empty_slots);
    sem_close(filled_slots);
    sem_unlink("/queue_mutex");
    sem_unlink("/empty_slots");
    sem_unlink("/filled_slots");
    munmap(print_queue, sizeof(PrintJob) * MAX_PRINT_JOBS);
    shm_unlink("/print_queue");

    return 0;
}
