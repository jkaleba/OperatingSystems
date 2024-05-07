#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/time.h>


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


int main(int argc, char *argv[]) {
    if (argc != 3) {
        perror("Usage: <step> <processes>\n");
        return 1;
    }

    int n = atoi(argv[2]);
    double h = atof(argv[1]), a = 0, b = 1, process_interval = (b - a) / n;

    if (h < 0 || h > 1 || n < 1) {
        perror("ERROR: argument must be a positive integer");
        return 1;
    }

    int fds[n][2];

    struct timeval start_time, end_time;
    gettimeofday(&start_time, NULL);

    for (int i = 0; i < n; i++) {

        if (pipe(fds[i]) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }
        pid_t pid = fork();

        if (pid == 0) {
            /* child process */
            close(fds[i][0]);

            double partial_result = integral(a + i * process_interval, a + (i + 1) * process_interval, h, f);

            write(fds[i][1], &partial_result, sizeof(partial_result));
            close(fds[i][1]);
            exit(EXIT_SUCCESS);
        }
        else if (pid < 0) {
            perror("ERROR: fork failed");
            exit(EXIT_FAILURE);
        }
    }

    double partial_result = 0, result = 0;
    for (int i = 0; i < n; i++) {
        read(fds[i][0], &partial_result, sizeof(partial_result));
        result += partial_result;

        close(fds[i][0]);
    }

    for (int i = 0; i < n; i++) {
        wait(NULL);
    }

    gettimeofday(&end_time, NULL);

    double elapsed = (end_time.tv_sec - start_time.tv_sec) + (end_time.tv_usec - start_time.tv_usec) / 1000000.0;

    printf("Calculated integral: %.15f\n", result);
    printf("Number of processes: %d\n", n);
    printf("Width of rectangles: %.15f\n", h);
    printf("Elapsed time: %f seconds\n", elapsed);

    FILE *file = fopen("report.txt", "w");
    if (file == NULL) {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
    fprintf(file, "Calculated integral: %.15f\n", result);
    fprintf(file, "Number of processes: %d\n", n);
    fprintf(file, "Width of rectangles: %.15f\n", h);
    fprintf(file, "Elapsed time: %f seconds\n", elapsed);
    fclose(file);

    return 0;
}