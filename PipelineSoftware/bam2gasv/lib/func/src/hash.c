#include <assert.h>
#include <stdlib.h>
#include "list.h"
#include "hash.h"
#include <stdio.h>

// CONSTRUCTORS

hash_t* hash(unsigned long size) {
  hash_t* hash = malloc(sizeof(hash_t));
  if (!hash) return NULL;
  hash->table = calloc(size, sizeof(list_t));
  if (!hash->table) {
    free(hash);
    return NULL;
  }
  hash->size = size;
  return hash;
}

unsigned int hash_free(hash_t* table, void* destructor) {
  unsigned int num_removed = hash_clear(table, destructor);
  free(table->table);
  free(table);
  return num_removed;
}


// ACCESSORS

void* hash_ref(hash_t* table, void* value, unsigned long hashcode, int (*cmp)(void*, void*)) {
  unsigned long index = hashcode % table->size;
  return list_find(&table->table[index], value, cmp);  
}

list_t *hash_bucket(hash_t *table, unsigned long hashcode) {
  return &table->table[hashcode % table->size];  
}

// MUTATORS

unsigned int hash_clear(hash_t* table, void* destructor) {

  int i, num_removed = 0;
  for (i=0; i<table->size; ++i) {
    num_removed += list_clear(&table->table[i], destructor);
    table->table[i].car = NULL;
    table->table[i].cdr = NULL;
  }
  return num_removed;
}

void hash_insert(hash_t* table, void* value, unsigned long hashcode, int (*cmp)(void*, void*), void* constructor) {

  unsigned long index = hashcode % table->size;
  list_merge_into(&table->table[index], list(1, constructor ? ((void*(*)(void*))constructor)(value) : value), cmp);
}

//TODO inline to optimize
void* hash_remsert(hash_t* table, void* value, unsigned long hashcode, int (*cmp)(void*, void*), void* constructor) {

  unsigned long index = hashcode % table->size;

  void *ret = list_remsert_sorted(&table->table[index], value, cmp, constructor);
  
  
  /*
  void* ret = list_remove(&table->table[index], value, cmp);  
  if (!ret) {
    list_merge_into(&table->table[index], list(1, constructor ? ((void*(*)(void*))constructor)(value) : value), cmp);
  }
  */
  
  return ret;

}

// HASHERS

//djb2 generic string hashing algorithm from http://www.cse.yorku.ca/~oz/hash.html
unsigned long djb2_hash(char *str) {
  assert(str);
  unsigned long hashed = 5381;
  int c;

  while ((c = *str++))
    hashed = ((hashed << 5) + hashed) + c; /* hashed * 33 + c */

  return hashed;
}
