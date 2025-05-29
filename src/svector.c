#include <stdio.h>
#include <stdlib.h>
#include "svector.h"

// This is perhaps the laziest way to implement this, I have two files, dvector and svector, dvector implements a vector of doubles,
// whereas svector implements a vector of slips, I could've wrote just one file that implements a vector of void pointers maybe,
// but I'm not smart enough for that.

SlipVector* screate_vector() {
    SlipVector* vector = (SlipVector*)malloc(sizeof(SlipVector));
    vector->data = NULL;
    vector->size = 0;
    vector->capacity = 0;
    return vector;
}

void spush_back(SlipVector* vector, Slip value) {
    if (vector->data == NULL) {
        vector->data = (Slip*)malloc(sizeof(Slip));
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = (Slip*)realloc(vector->data, vector->capacity * sizeof(Slip));
    }

    vector->data[vector->size] = value;
    vector->size++;
}

void spop_back(SlipVector* vector) {
    if (vector->size > 0)
        vector->size--;
}

Slip svector_at(SlipVector* vector, size_t index) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(1);
    }
    return vector->data[index];
}

void sdestroy(SlipVector* vector) {
    free(vector->data); free(vector);
}