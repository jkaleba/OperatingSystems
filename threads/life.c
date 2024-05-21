#include <ncurses.h>
#include <locale.h>
#include <unistd.h>
#include "grid.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>


int main(int argc, char **argv) {
    srand(time(NULL));
    setlocale(LC_CTYPE, "");
    initscr(); // Start curses mode

    if (argc != 4) {
        printf("Usage: %s <number of threads> <map width> <map height>\n", argv[0]);
        return 1;
    }
    int n = atoi(argv[1]);
    int grid_width = atoi(argv[2]);
    int grid_height = atoi(argv[3]);

    if (n < 1 || grid_width < 1 || grid_height < 1) {
        printf("Invalid arguments\n");
        return 1;
    }

    if (n > grid_width * grid_height) {
        n = grid_width * grid_height;
    }

    char *foreground = create_grid(grid_width, grid_height);
    char *background = create_grid(grid_width, grid_height);
    char *tmp;

    init_grid(foreground);

    pthread_t threads[n];

    int cells_per_thread = grid_width * grid_height / n;
    int cells_remaining = grid_width * grid_height % n;

    thread_data_t thread_data[n];

    sigset_t signal_set;
    sigemptyset(&signal_set);
    sigaddset(&signal_set, SIGUSR1);

    pthread_sigmask(SIG_BLOCK, &signal_set, NULL);

    for (int i = 0; i < n; i++) {
        thread_data[i].thread_id = i;
        thread_data[i].start = i * cells_per_thread;
        thread_data[i].end = (i == n - 1) ? (i * cells_per_thread + cells_per_thread + cells_remaining) : (i * cells_per_thread + cells_per_thread);
        thread_data[i].foreground = foreground;
        thread_data[i].background = background;
        thread_data[i].signal_set = &signal_set;
        pthread_create(&threads[i], NULL, update_grid, &thread_data[i]);
    }

    int max_iter = 10000;

    for (int iter = 0; iter < max_iter; iter++) {
        draw_grid(foreground);
        usleep(500 * 1000);

        // Step simulation
        for (int i = 0; i < n; i++) {
            pthread_kill(threads[i], SIGUSR1);
        }

        tmp = foreground;
        foreground = background;
        background = tmp;
        for (int i = 0; i < n; i++) {
            thread_data[i].foreground = foreground;
            thread_data[i].background = background;
        }
    }

    endwin(); // End curses mode
    destroy_grid(foreground);
    destroy_grid(background);

    return 0;
}
