#ifndef FUNC_LIST_H
#define FUNC_LIST_H

// DATATYPES

typedef struct list_t {
  struct list_t* cdr;
  void* car;  
} list_t;

// CONSTRUCTORS

list_t* list(int, ...);
unsigned int list_free(list_t*, void*);
list_t *list_copy(list_t *, void *);
void list_buffer_free(list_t *, void *);
void alist_free(list_t *, void *, void *);
list_t* cons(void*, list_t*);
void cons_init(list_t *, void *, list_t *);
list_t* pair(void*, void*);
void cons_free(list_t*, void*, void*);

// PREDICATES

int list_cmp(list_t*, list_t*, void*);
int alist_cmp(list_t *, list_t *, void *, void *);
#define list_empty(lst) (!(lst)->car && !(lst)->cdr)
int list_sorted(list_t *, int (*)(void *, void *));

// ACCESSORS

void* list_ref(list_t*, int);
void* alist_ref(list_t *, void *, void *);
list_t *list_tail(list_t*, int);
list_t *list_tailf(list_t*, void*, void*);
void* list_find(list_t*, void*, void*);
list_t* list_findall(list_t*, void*, void*);
int list_index_of(list_t *, void* , void *);
int list_length(list_t*);

// MUTATORS

unsigned int list_clear(list_t* lst, void* destructor);
list_t *list_reverse(list_t *);
list_t *list_append(list_t *, list_t *);
void list_insert(list_t*, void*, int);
void list_insert_unique(list_t *, void *, void *);
list_t *list_taild(list_t *, int, void *);
void* list_remove(list_t*, void*, int (*)(void*, void*));
void* list_remove_sorted(list_t* , void* , int (*)(void*, void*));
void* list_remsert_sorted(list_t* , void* , int (*)(void*, void*), void *);
list_t *list_dropf(list_t*, void*, void*);

// hIGHER ORDER

void list_values(list_t *, ...);
list_t *list_map(list_t *, void*, list_t *);

// THREADING

void list_buffer_write(volatile list_t *, void *);
list_t * list_buffer_read(volatile list_t *);
list_t * list_buffer_readn_try(volatile list_t *, int);
list_t * list_buffer_read_try(volatile list_t *);

// SORTING
void list_merge_into(list_t*, list_t*, int (*)(void*, void*));
list_t* list_mergesort(list_t *, int (*)(void*, void*));
list_t *list_merge(list_t *, list_t *, int (*)(void*, void*));

// COERSION

void** list2array(list_t*);

// I/O

void list_print(list_t*, void(*)(void*));
void alist_print(list_t *, void(*)(void*), void(*)(void*));

#endif
