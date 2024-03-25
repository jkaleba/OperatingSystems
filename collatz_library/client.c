#include <stdio.h>


#ifdef DYNAMIC_DLOPEN
    #include <dlfcn.h>

    int (*collatz_conjecture)(int input);
    int (*test_collatz_convergence)(int input, int max_iter);
#endif

#ifndef DYNAMIC_DLOPEN
    #include "collatz.h"
#endif

int main() {

#ifdef DYNAMIC_DLOPEN
    void *dll_handle = dlopen("./l_collatz.so", RTLD_LAZY);
    if (!dll_handle) {
        fprintf(stderr, "Error: %s\n", dlerror());
        return 1;
    }

    collatz_conjecture = dlsym(dll_handle, "collatz_conjecture");
    test_collatz_convergence = dlsym(dll_handle, "test_collatz_convergence");
#endif

    int conj = collatz_conjecture(5);
    int cov = test_collatz_convergence(5, 1000);

    printf("%d\n", conj);
    printf("%d\n", cov);

    return 0;
}