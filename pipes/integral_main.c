#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define FIFO_PATH "integral"

int main() {
    double a, b;
    int fifo_fd;
    double result;

    if (mkfifo(FIFO_PATH, 0666) == -1) {
        perror("mkfifo");
        exit(EXIT_FAILURE);
    }

    printf("Podaj przedział całkowania [a, b]: ");
    scanf("%lf %lf", &a, &b);

    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    write(fifo_fd, &a, sizeof(double));
    write(fifo_fd, &b, sizeof(double));
    close(fifo_fd);

    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    read(fifo_fd, &result, sizeof(double));
    printf("Wartość całki w przedziale [%.2lf, %.2lf] wynosi: %.6lf\n", a, b, result);
    close(fifo_fd);

    unlink(FIFO_PATH);

    return 0;
}
