#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "heap.h"
#include "array.h"

// HELPERS 

#define heap_parent(i) (((i) - 1) / 2)
#define heap_left(i) (2 * (i) + 1)
#define heap_right(i) (2 * (i) + 2)

static inline void swap(heap_t *h, int left, int right) {
  void *s = h->data[left];
  h->data[left] = h->data[right];
  h->data[right] = s;
}

// CONSTRUCTORS

heap_t* heap(int capacity) {
  heap_t* a = malloc(sizeof(heap_t));
  if (!a) return NULL;
  a->capacity = capacity;
  a->data = calloc(capacity, sizeof(void*));
  if (!a->data) {
    free(a);
    return NULL;
  }
  a->size = 0;
  return a;
}


void heap_free(heap_t *a, void *destructor) {
  int i;
  if (destructor) {
    for (i=0; i<a->size; i++) {
      if (a->data[i]) ((void(*)(void*))destructor)(a->data[i]);
    }
  }
  free(a->data);
  free(a);
}

// ACCESSORS

void *heap_peek(heap_t *h) {
  return 0 < h->size ? h->data[0] : NULL;
}

// MUTATORS

void heap_insert(heap_t* h, void* value, int (*cmp)(void*, void*)) {
  assert(h->size < h->capacity);
  int curr = h->size++;
  h->data[curr] = value;

  while (curr && 0 > cmp(value, h->data[heap_parent(curr)])) {
    swap(h, curr, heap_parent(curr));
    curr = heap_parent(curr);
  }  
}

void heap_reheap_down(heap_t *h, int (*cmp)(void*, void*)) {
  int min = 0, parent = 0;
  //while parent greater than either child
  while (1) {
    if (heap_left(parent) < h->size && cmp(h->data[heap_left(parent)], h->data[parent]) <= 0) {
      min = heap_left(parent);
    }
    if (heap_right(parent) < h->size && cmp(h->data[heap_right(parent)], h->data[min]) <= 0) {
      min = heap_right(parent);
    }
    if (min != parent) {
      swap(h, min, parent);
      parent = min;
    }
    else break;
  }
}

void heap_delete(heap_t *h, int (*cmp)(void*, void*), void *destructor) {
  assert(0 < h->size);
  if (destructor) ((void(*)(void*))destructor)(h->data[0]);
  h->size--;
  h->data[0] = h->data[h->size];
  heap_reheap_down(h, cmp);  
}

void heap_print(heap_t *h, void(*p)(void*)) {
  int i;
  printf("[");
  for (i=0; i<h->size; i++) {
    p(h->data[i]);
    if (i < h->size - 1) printf(" ");
  }
  printf("]");
}

/*
void heap_delete(heap_t* h, int (*cmp)(void*, void*)) {
  if (h->size-- > 1) heap_delsert(h, array_get((array_t*)h, h->size), cmp);
}

void heap_delsert(heap_t* h, void* value, int (*cmp)(void*, void*)) {
  int curr = 0;
  int min_child;
  
  array_set((array_t*)h, 0, value);

  while ((min_child = heap_left(curr)) < h->size) {
    if (min_child + 1 < h->size && 0 < cmp(array_get((array_t*)h, min_child), array_get((array_t*)h, heap_right(curr)))) {
      min_child = heap_right(curr);
    }

    if (0 < cmp(value, array_get((array_t*)h, min_child))) {
      array_swap((array_t*)h, curr, min_child);
      curr = min_child;
    }
    else {
      break;
    }
  }
}
*/
