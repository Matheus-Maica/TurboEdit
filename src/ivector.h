#ifndef IVECTOR_H
#define IVECTOR_H

#include <stdlib.h> // for size_t

typedef struct {
    int* data;
    size_t size;
    size_t capacity;
} IVector;

IVector* icreate_vector();
void ipush_back(IVector* vector, int value);
void ipop_back(IVector* vector);
int ivector_at(IVector* vector, size_t index);
size_t isize(IVector* vector);
int iis_empty(IVector* vector);
void idestroy(IVector* vector);
void iprint_vector(IVector* vector);

#endif // VECTOR_H