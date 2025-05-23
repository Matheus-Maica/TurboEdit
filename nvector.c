#include <stdio.h>
#include <stdlib.h>
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

size_t size(Vector* vector) {
    return vector->size; // Return the size of the vector
}

int is_empty(Vector* vector) {
    return vector->size == 0; // Return 1 if the vector is empty, 0 otherwise
}