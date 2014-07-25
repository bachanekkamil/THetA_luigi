#include <stdlib.h>
#include <string.h>
#include "array.h"

// CONSTRUCTORS

array_t *array(int capacity, int data_size) {
  array_t *a = malloc(sizeof(array_t));
  a->capacity = capacity;
  a->data_size = data_size;
  a->data = calloc(capacity, data_size);
  return a;
}

void array_free(array_t *a, void *destructor) {
  int i;
  for (i=0; i<a->capacity; i++) {
    
  }
  free(a->data);
  free(a);
}

// ACCESSORS

void* array_get(array_t* a, int index) {
  return a->data + (a->data_size * index);
}

// MUTATORS

void array_set(array_t* a, int index, void* value) {
  memcpy(a->data + (a->data_size * index), value, a->data_size);
}

void array_swap(array_t* a, int left, int right) {
  char swap[a->data_size / sizeof(char)];
  memcpy(&swap, array_get(a, left), a->data_size);
  array_set(a, left, array_get(a, right));
  array_set(a, right, &swap);
}

