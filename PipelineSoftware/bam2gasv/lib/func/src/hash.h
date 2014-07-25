#ifndef FUNC_HASH_H
#define FUNC_HASH_H

#include "list.h"

// DATATYPES

typedef struct {
  list_t* table;
  unsigned long size;
} hash_t;

// CONSTRUCTORS

hash_t* hash(unsigned long);
unsigned int hash_free(hash_t*, void*);

// ACCESSORS

void* hash_ref(hash_t*, void* , unsigned long, int (*)(void*, void*));
list_t *hash_bucket(hash_t *, unsigned long);

// MUTATORS

unsigned int hash_clear(hash_t*, void*);
void hash_insert(hash_t* , void* , unsigned long , int (*)(void*, void*), void* );
void* hash_remsert(hash_t*, void*, unsigned long, int (*)(void*, void*), void*);

// HASHERS

unsigned long djb2_hash(char*);

#endif
