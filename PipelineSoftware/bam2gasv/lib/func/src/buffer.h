#ifndef FUNC_BUFFER_H
#define FUNC_BUFFER_H

#include <pthread.h>

#define full2n(b, n) (((b)->read_from + n < (b)->capacity && ((b)->write_to < (b)->read_from || (b)->read_from + n < (b)->write_to)) || ((b)->capacity <= (b)->read_from + n && ((b)->write_to < (b)->read_from && (b)->read_from + n < (b)->write_to + (b)->capacity)))

// DATATYPES

typedef struct {
  int capacity;
  int data_size;
  void* data;
  int volatile read_from;
  int volatile write_to;  
  pthread_mutex_t mutex;
  pthread_cond_t cond;
} buffer_t;

// CONSTRUCTORS 

buffer_t* buffer(int, int);
void buffer_free(buffer_t*, void*);

// PREDICATES

#define buffer_empty(b) ((b)->read_from == (b)->write_to)
#define buffer_full(b) (((b)->write_to + 1) % (b)->capacity == (b)->read_from)
#define buffer_one(b) (((b)->write_to + 2) % (b)->capacity == (b)->read_from)

// READING

void buffer_read(buffer_t *, void *);
int buffer_read_try(buffer_t *, void *);
void* buffer_peek(buffer_t*);
void* buffer_peekn(buffer_t *, int);
void* buffer_peek_try(buffer_t*);
void* buffer_peekn_try(buffer_t*, int);
void* buffer_peek_lock(buffer_t *);
void buffer_consume(buffer_t*);

// WRITING

void buffer_write(buffer_t*, void*);
void buffer_write_unlock(buffer_t *, void *);
void* buffer_buffer(buffer_t*);
void* buffer_buffer_try(buffer_t*);
void buffer_flush(buffer_t*);
void buffer_flush_unlock(buffer_t*);
void buffer_flush_unlock_unsafe(buffer_t*);

#endif
