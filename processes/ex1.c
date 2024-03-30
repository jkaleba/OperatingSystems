#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>


int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Need exactly 1 argument.");
        return 1;
    }

    const char* arg = argv[1];

    int n = atoi(arg);

    for (int i = 0; i < n; i++) {

        pid_t child_pid = fork();

        if (child_pid == 0) {
            printf("Proces dziecka: Proces rodzica ma pid:%d\n",(int)getppid());
            printf("Proces dziecka: Proces dziecka ma pid:%d\n",(int)getpid());
            exit(0);
        } else if (child_pid < 0){
            printf("fork() failed\n");
            return 1;
        }

    }

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    printf("\n%d\n", n);

    return 0;
}
