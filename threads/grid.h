#pragma once
#include <stdbool.h>
#include <pthread.h>
#include <signal.h>


char *create_grid(int width, int height);
void destroy_grid(char *grid);
void draw_grid(char *grid);
void init_grid(char *grid);
bool is_alive(int row, int col, char *grid);

_Noreturn void *update_grid(void *args);


typedef struct {
    int thread_id;
    int start;
    int end;
    char *foreground;
    char *background;
    pthread_t thread;
    sigset_t *signal_set;
} thread_data_t;