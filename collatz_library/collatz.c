#include "collatz.h"


int collatz_conjecture(int n) {
    return n % 2 == 0 ? n / 2 : 3 * n + 1;
}

int test_collatz_convergence(int input, int max_iter) {
    int iterations = 0;
    while (input != 1 && iterations < max_iter) {
        input = collatz_conjecture(input);
        iterations++;
    }

    return iterations != max_iter ? iterations : -1;
}