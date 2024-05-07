#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>

#define FIFO_PATH "integral"
#define H 0.0001

double f(double x) {
    return 4 / (x * x + 1);
}

double integral(double a, double b, double h, double f(double)) {
    double sum = 0;
    int n = (int)((b - a) / h);
    for (int i = 0; i < n; i++) {
        sum += f(a + i * h) + f(a + (i + 1) * h);
    }

    return h * sum / 2;
}


int main() {

    double a, b;
    int fifo_fd;
    double result;

    fifo_fd = open(FIFO_PATH, O_RDONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    read(fifo_fd, &a, sizeof(double));
    read(fifo_fd, &b, sizeof(double));
    close(fifo_fd);

    result = integral(a, b, H, f);

    fifo_fd = open(FIFO_PATH, O_WRONLY);
    if (fifo_fd == -1) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    write(fifo_fd, &result, sizeof(double));
    close(fifo_fd);

    return 0;
}