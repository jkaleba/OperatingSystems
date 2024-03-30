#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>


int global = 0;

int main(int argc, char* argv[]) {

    if (argc != 1) {
        printf("Need exactly 1 argument.");
        return 1;
    }

    printf("%s\n", argv[0]);

    int local = 0;

    const char* destination = argv[1];

    pid_t returned_pid = fork();

    if (returned_pid == 0) {
        global++, local++;
        printf("Child process.");
        printf("Child PID = %d, Parent PID = %d", (int)getpid(), (int)getppid());

        int err = execl("/bin/ls", "ls", destination, NULL);
        exit(err);

    } else if (returned_pid > 0) {
        printf("Parent process.");
        printf("Parent PID = %d, Child PID = %d", (int)getpid(), (int)returned_pid);

        int stat_loc = 0;
        pid_t w_pid = waitpid(returned_pid, &stat_loc, 0);

        int status = WEXITSTATUS(stat_loc);
        printf("Child process exited with status: %d", status);

        printf("Parent's local = %d, parent's global = %d\n", local, global);

        return status;

    } else {
        printf("fork() failed\n");
        return 1;
    }
}
