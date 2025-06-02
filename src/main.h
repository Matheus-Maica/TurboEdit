#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>

typedef struct {
    SlipVector* arcs;
    size_t widelane_arcs_length;
    double* ionospheric;
    int ionospheric_slips_length;
    int* outliers;
    int outliers_length;
} Results;

Results find_cycle_slips(double* a, double* b, double* c, double* d, size_t n);

#endif