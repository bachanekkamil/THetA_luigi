#include <assert.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <sched.h>
#include "buffer.h"

//Provides a lock-free, fixed-size, single-reader, single-writer, thread-safe communication channel for fixed-size messages.
 


// CONSTRUCTORS 

buffer_t* buffer(int capacity, int data_size) {
  assert(0 < capacity && 0 < data_size);
  capacity = capacity + 1; //Queue is full when read and write are offset by 1, so to add n objects to a size n queue, we need capacity n + 1.
  buffer_t* b = malloc(sizeof(buffer_t));
  if (!b) return NULL;
  b->data_size = data_size;
  b->capacity = capacity;
  b->read_from = 0;
  b->write_to = 0;
  b->data = calloc(capacity, data_size);  
  if (!b->data) {
    free(b);
    return NULL;
  }
  pthread_mutex_init(&b->mutex, NULL);
  pthread_cond_init(&b->cond, NULL);
  return b;
}


void buffer_free(buffer_t* b, void* message_free) {
  if (!b) return;
  assert(free != message_free); // message_free is passed a pointer to each filled cell of the data array. These must be dereferenced before being freed.

  int i;
  if (message_free) {
    //for (i=b->read_from; i!=b->write_to; i=(i+1)%b->capacity) {
    for (i=0; i<b->capacity; i++) {
      ((void (*)(void*))message_free)(b->data + (b->data_size * i));
    }
  }
  free(b->data);
  pthread_mutex_destroy(&b->mutex);
  pthread_cond_destroy(&b->cond);
  free(b);
}


// READING

void buffer_read(buffer_t *b, void *dest) {
  assert(b && dest);
  memcpy(dest, buffer_peek(b), b->data_size);
  bzero(buffer_peek(b), b->data_size);
  buffer_consume(b);
}

int buffer_read_try(buffer_t *b, void *dest) {
  assert(b && dest);
  void *msg = buffer_peek_try(b);
  if (!msg) return 0;
  memcpy(dest, msg, b->data_size);
  bzero(buffer_peek(b), b->data_size);
  buffer_consume(b);
  return 1;
}

void* buffer_peekn(buffer_t *b, int n) {
  assert(b && 0 <= n);
  assert(n < b->capacity);
  //while ((b->read_from % b->capacity < b->write_to % b->capacity) && (b->write_to % b->capacity < (b->read_from + n + 1) % b->capacity)) 

  while (!full2n(b, n)) sched_yield();
  return b->data + (b->data_size * ((b->read_from + n) % b->capacity));
}

void* buffer_peek(buffer_t* b) {
  return buffer_peekn(b, 0);
}

void* buffer_peekn_try(buffer_t* b, int n) {
  assert(n < b->capacity);
  return full2n(b, n) ?
    buffer_peekn(b, n) :
    NULL;
}

void* buffer_peek_try(buffer_t* b) {
  return buffer_peekn_try(b, 0);
}

void* buffer_peek_lock(buffer_t *b) {
  assert(b);
  int n = 0;
  
  if (!full2n(b, n)) {
    pthread_mutex_lock(&b->mutex);
    while (!full2n(b, n)) pthread_cond_wait(&b->cond, &b->mutex);
    pthread_mutex_unlock(&b->mutex);
  }
  return b->data + (b->data_size * ((b->read_from + n) % b->capacity));
}

/*
pthread_mutex_lock(&b->mutex);
    while (!full2n(b, n)) pthread_cond_wait(&b->cond, &b->mutex);
    pthread_mutex_unlock(&b->mutex);
  return b->data + (b->data_size * ((b->read_from + n) % b->capacity));
*/

void buffer_consume(buffer_t* b) {
  assert(b);
  while (b->read_from == b->write_to) sched_yield();
  b->read_from = (b->read_from + 1) % b->capacity;
}

// WRITING

void buffer_write(buffer_t* b, void* msg) {
  assert(b);
  while (buffer_full(b)) sched_yield(); 
  memcpy(b->data + (b->data_size * b->write_to), msg, b->data_size);
  b->write_to = (b->write_to + 1) % b->capacity;
}

void buffer_write_unlock(buffer_t* b, void *msg) {
  buffer_write(b, msg);
  pthread_cond_signal(&b->cond);
}

void* buffer_buffer(buffer_t* b) {
  assert(b);
  while buffer_full(b) sched_yield(); 
  return b->data + (b->data_size * b->write_to);
}

void* buffer_buffer_try(buffer_t* b) {
  return buffer_full(b) ? 
    NULL : 
    b->data + (b->data_size * b->write_to);
}

void buffer_flush(buffer_t* b) {
  if (!b) return;
  while buffer_full(b) sched_yield(); 

  b->write_to = (b->write_to + 1) % b->capacity;
}

void buffer_flush_unlock(buffer_t* b) {
  if (!b) return;
  while buffer_full(b) sched_yield(); 

  b->write_to = (b->write_to + 1) % b->capacity;
  pthread_cond_signal(&b->cond);
}

void buffer_flush_unlock_unsafe(buffer_t* b) {
  if (!b) return;
  while buffer_full(b) sched_yield(); 
    
  int consumer_waiting = buffer_empty(b); // may be false even though consumer is waiting.
  b->write_to = (b->write_to + 1) % b->capacity;
  if (consumer_waiting) pthread_cond_signal(&b->cond);
}
