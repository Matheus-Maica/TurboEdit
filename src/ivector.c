#include <stdio.h>
#include <stdlib.h>
#include "ivector.h"

IVector* icreate_vector() {
    IVector* vector = (IVector*)malloc(sizeof(IVector));
    vector->data = NULL;   // Set data pointer to NULL, indicating an empty vector
    vector->size = 0;     // Initialize the size to 0 (no elements in the vector)
    vector->capacity = 0; // Initialize the capacity to 0 (no memory allocated)
    return vector; // Return the newly created vector
}

void ipush_back(IVector* vector, int value) {
    if (vector->data == NULL) {
        vector->data = (int*)malloc(sizeof(int));
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = (int*)realloc(vector->data, vector->capacity * sizeof(int));
    }

    vector->data[vector->size] = value;
    vector->size++;
}

void ipop_back(IVector* vector) {
    if (vector->size > 0)
        vector->size--;
}

int ivector_at(IVector* vector, size_t index) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(1);
    }
    return vector->data[index];
}

size_t isize(IVector* vector) {
    return vector->size;
}

int iis_empty(IVector* vector) {
    return vector->size == 0;
}

void idestroy(IVector* vector) {
    free(vector->data); free(vector);
}

void iprint_vector(IVector* vector) {
    printf("IVector (size: %zu, capacity: %zu): [", vector->size, vector->capacity);
    for (size_t i = 0; i < vector->size; ++i) {
        printf("%d", vector->data[i]);
        if (i < vector->size - 1)
            printf(", ");
    }
    printf("]\n");
}