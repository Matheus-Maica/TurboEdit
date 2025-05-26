#include <stdio.h>
#include <stdlib.h>
#include <cblas.h>
#include "nvector.h"

Vector* create_vector() {
    Vector* vector = (Vector*)malloc(sizeof(Vector));
    vector->data = NULL;   // Set data pointer to NULL, indicating an empty vector
    vector->size = 0;     // Initialize the size to 0 (no elements in the vector)
    vector->capacity = 0; // Initialize the capacity to 0 (no memory allocated)
    return vector; // Return the newly created vector
}

void push_back(Vector* vector, double value) {
    if (vector->data == NULL) {
        // If the vector is empty, allocate memory for one element
        vector->data = (double*)malloc(sizeof(double));
        vector->capacity = 1;
    } 
    else if (vector->size >= vector->capacity) {
        // If the vector is full, double its capacity by reallocating memory
        vector->capacity *= 2;
        vector->data = (double*)realloc(vector->data, vector->capacity * sizeof(double));
    }
    // Add the new element to the end of the vector and increment the size
    vector->data[vector->size] = value;
    vector->size++;
}

void pop_back(Vector* vector) {
    if (vector->size > 0) {
        // If the vector is not empty, decrement the size to remove the last element
        vector->size--;
    }
}

double vector_at(Vector* vector, size_t index) {
    if (index >= vector->size) {
        // Check if the provided index is out of bounds
        fprintf(stderr, "Index out of bounds\n"); // Print an error message
        exit(1); // Exit the program with an error code
    }
    return vector->data[index]; // Return the element at the specified index
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

    // Use cblas_dcopy if available
    cblas_dcopy((int)length, array, 1, vector->data, 1);
}

size_t size(Vector* vector) {
    return vector->size; // Return the size of the vector
}

int is_empty(Vector* vector) {
    return vector->size == 0; // Return 1 if the vector is empty, 0 otherwise
}

void destroy(Vector* vector) {
    free(vector->data);
    free(vector);
}

void print_vector(Vector* vector) {
    if (!vector) {
        printf("Vector is NULL.\n");
        return;
    }

    printf("Vector (size: %zu, capacity: %zu): [", vector->size, vector->capacity);
    for (size_t i = 0; i < vector->size; ++i) {
        printf("%.2f", vector->data[i]);
        if (i < vector->size - 1)
            printf(", ");
    }
    printf("]\n");
}