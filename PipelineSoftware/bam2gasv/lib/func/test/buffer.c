#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "buffer.h"

void pointer_free(void*** ptr) {
  if (*ptr) free(*ptr);
}

int main() {
  int* msg = malloc(sizeof(int));
  int* msg2 = malloc(sizeof(int));
  int* msg3 = malloc(sizeof(int));
  int* msg4 = malloc(sizeof(int));
  *msg = 1;
  *msg2 = 2;
  *msg3 = 3;
  *msg4 = 4;
  int *read;

  // Initializing.
  buffer_t* b = buffer(3, sizeof(int*));

  // Writing.
  assert(buffer_empty(b) && !buffer_full(b));
  assert(!buffer_peek_try(b));
  assert(!buffer_peekn_try(b, 2));
  buffer_write(b, &msg);
  buffer_write(b, &msg2);
  memcpy(buffer_buffer(b), &msg3, sizeof(int*));
  buffer_flush(b);
  assert(buffer_full(b) && !buffer_empty(b));
  
  // Reading
  assert(**(int**)buffer_peek(b) == 1);
  assert(**(int**)buffer_peekn_try(b, 0) == 1);
  assert(**(int**)buffer_peekn(b, 0) == 1);
  assert(**(int**)buffer_peekn(b, 1) == 2);
  assert(**(int**)buffer_peekn(b, 2) == 3);
  assert(**(int**)buffer_peekn_try(b, 2) == 3);

  buffer_read(b, &read);
  assert(1 == *read);
  assert(**(int**)buffer_peekn(b, 1) == 3);
  assert(!buffer_peekn_try(b, 2));

  buffer_write(b, &msg4);
  assert(**(int**)buffer_peekn(b, 2) == 4);

  /*
  //free(*(void**)b->data);  
  buffer_consume(b);
  printf("read from: %d, write to: %d, is empty: %d\n", b->read_from, b->write_to, buffer_empty(b));
  buffer_write(b, &msg);
  assert(**(int**)buffer_peek(b) == 43);
  assert(**(int**)buffer_peekn(b, 1) == 42);
  */

  // Destroying.
  buffer_free(b, pointer_free);
  free(msg);
  

  return 0;
}
