#ifndef MAIN_H
#define MAIN_H

#include <stddef.h>

typedef struct {
    SlipVector* arcs;
    int* outliers;
    int outliers_length;
} WlData; // wide-lane results

typedef struct {
    WlData widelane;
    double* ionospheric;
    int widelane_arcs_length;
    int ionospheric_slips_length;
    int outliers_length;
} Results;

Results* find_cycle_slips(double* a, double* b, double* c, double* d, size_t n);

#endif