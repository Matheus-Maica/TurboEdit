#ifndef SVECTOR_H
#define SVECTOR_H

#include <stdlib.h>

typedef struct {
    int index;
    double mean_bw;
    double stdev;
    int delta_N_w;
    int nPoints;
    char isPhaseConnected;
} Slip;

typedef struct {
    Slip* data;
    size_t size;
    size_t capacity;
} SlipVector;

SlipVector* screate_vector();
void spush_back(SlipVector* vector, Slip value);
void spop_back(SlipVector* vector);
Slip svector_at(SlipVector* vector, size_t index);
void sdestroy(SlipVector* vector);
Slip* svector_at_ptr(SlipVector* vector, size_t index);

#endif