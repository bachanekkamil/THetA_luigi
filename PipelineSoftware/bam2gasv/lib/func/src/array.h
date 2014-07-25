#ifndef FUNC_ARRAY_H
#define FUNC_ARRAY_H

// DATATYPES

typedef struct {
  int capacity;
  int data_size;
  void* data;
} array_t;

// CONSTRUCTORS

array_t *array(int, int);
void array_free(array_t *, void *);

// ACCESSORS

void* array_get(array_t*, int);

// MUTATORS

void array_set(array_t*, int, void*);
void array_swap(array_t*, int, int);

#endif
