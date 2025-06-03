#include "../src/main.h"
#include "../src/nvector.h"
#include <stdlib.h>
#include <stdio.h>

int main() {
    size_t n = 50000000;
    double* a = malloc(sizeof(double) * n);
    double* b = malloc(sizeof(double) * n);
    double* c = malloc(sizeof(double) * n);
    double* d = malloc(sizeof(double) * n);

    for (size_t i = 0; i < n; i++) {
        a[i] = i * 0.1;
        b[i] = i * 0.2;
        c[i] = i * 0.3;
        d[i] = 0;
    }

    fprintf(stderr, "TESTING STDERR\n");
    fflush(stderr);

    find_cycle_slips(a, b, c, d, n);

    free(a); free(b); free(c); free(d);
    return 0;
}