#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "dvector.h"

Vector* create_vector() {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    vector->data = NULL;   // Set data pointer to NULL, indicating an empty vector
    vector->size = 0;     // Initialize the size to 0 (no elements in the vector)
    vector->capacity = 0; // Initialize the capacity to 0 (no memory allocated)
    return vector; // Return the newly created vector
}

void push_back(Vector* vector, double value) {
    if (vector->data == NULL) {
        vector->data = (double*)malloc(sizeof(double));
        vector->capacity = 1;
    } else if (vector->size >= vector->capacity) {
        vector->capacity *= 2;
        vector->data = (double*)realloc(vector->data, vector->capacity * sizeof(double));
    }

    vector->data[vector->size] = value;
    vector->size++;
}

void pop_back(Vector* vector) {
    if (vector->size > 0)
        vector->size--;
}

double vector_at(Vector* vector, size_t index) {
    if (index >= vector->size) {
        fprintf(stderr, "Index out of bounds\n");
        exit(1);
    }
    return vector->data[index];
}

void copy_from_array(Vector* vector, double* array, size_t length) {
    if (!vector) return;

    vector->data = (double*)malloc(length * sizeof(double));
    if (!vector->data) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    vector->capacity = length;
    vector->size = length;

    cblas_dcopy((int)length, array, 1, vector->data, 1);
}

size_t size(Vector* vector) {
    return vector->size;
}

int is_empty(Vector* vector) {
    return vector->size == 0;
}

void destroy(Vector* vector) {
    free(vector->data); free(vector);
}

void print_vector(Vector* vector) {
    printf("Vector (size: %zu, capacity: %zu): [", vector->size, vector->capacity);
    for (size_t i = 0; i < vector->size; ++i) {
        printf("%.2f", vector->data[i]);
        if (i < vector->size - 1)
            printf(", ");
    }
    printf("]\n");
}