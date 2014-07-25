#ifndef FUNC_HEAP_H
#define FUNC_HEAP_H

// DATATYPES

typedef struct {
  int capacity;
  void **data;
  int size;
} heap_t;

// CONSTRUCTORS

heap_t* heap(int);
void heap_free(heap_t *, void *);

// ACCESSORS

void *heap_peek(heap_t *);

// MUTATORS

void heap_insert(heap_t*, void*, int (*)(void*, void*));
void heap_delete(heap_t *, int (*)(void*, void*), void *);
void heap_reheap_down(heap_t *, int (*)(void*, void*));

// I/O

void heap_print(heap_t *, void(*)(void*));

#endif
