#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "heap.h"
#include "array.h"

int int_cmp(void* a, void* b) {
  if (!a || !b) return 0;
  return *(int*)a - *(int*)b;
}

void int_print(void* i) {
  if (i) printf("%d", *(int*)i);
}

int main() {

  int n[] = {0, 1, 2, 3, 4};
  int *num = malloc(sizeof(int));
  int *num2 = malloc(sizeof(int));
  *num = 3;
  *num2 = 4;

  heap_t *h = heap(4);
  
  //test heap_insert
  heap_insert(h, &n[1], int_cmp);
  heap_insert(h, &n[2], int_cmp);
  heap_insert(h, &n[0], int_cmp);  
  assert(&n[0] == heap_peek(h));
  heap_delete(h, int_cmp, NULL);
  assert(&n[1] == heap_peek(h));
  heap_delete(h, int_cmp, NULL);
  assert(&n[2] == heap_peek(h));
  heap_delete(h, int_cmp, NULL);
  assert(NULL == heap_peek(h));
  heap_free(h, NULL);

  // test heap_free
  h = heap(2);
  heap_insert(h, num, int_cmp);
  heap_insert(h, num2, int_cmp);
  assert(num == heap_peek(h));
  *num = 5;
  heap_reheap_down(h, int_cmp);
  assert(num2 == heap_peek(h));
  heap_delete(h, int_cmp, free);
  heap_reheap_down(h, int_cmp);
  heap_free(h, free);

  
  return 0;
}
