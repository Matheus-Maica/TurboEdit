#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h> // for size_t

typedef struct {
    double* data;
    size_t size;
    size_t capacity;
} Vector;

Vector* create_vector();
void push_back(Vector* vector, double value);
void pop_back(Vector* vector);
double vector_at(Vector* vector, size_t index);
size_t size(Vector* vector);
int is_empty(Vector* vector);
void copy_from_array(Vector* vector, double* array, size_t length);
void destroy(Vector* vector);
void print_vector(Vector* vector);

#endif // VECTOR_H