#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <sched.h>
#include "list.h"
#include "fn.h"

// HELPERS

int compare(void *val1, void *val2, void *cmp) {
  if (cmp) {
    return ((int (*)(void *, void *))cmp)(val1, val2);
  }
  else return val1 != val2;
}

#define call_maybe(fn, arg) ((fn) ? ((void *(*)(void *))fn)((arg)) : (arg))

// CONSTRUCTORS

list_t* list(int len, ...) {
  assert(0 <= len);
  if (!len) return calloc(1, sizeof(list_t));

  va_list args;
  list_t *lst = NULL, *next, **cdr;
  cdr = &lst;
  
  va_start(args, len);
  do {
    next = calloc(1, sizeof(list_t));
    if (!next) {
      list_free(lst, NULL);
      return NULL;
    }
    next->car = len ? va_arg(args, void*) : NULL;
    *cdr = next;
    cdr = &(next->cdr);
  } while(--len);
  va_end(args);
  return lst;
}

list_t* cons(void* car, list_t* cdr) {
  list_t* c = malloc(sizeof(list_t));
  if (!c) return NULL;
  c->car = car;
  c->cdr = cdr;
  return c;
}

void cons_init(list_t *c, void *car, list_t *cdr) {
  c->car = car;
  c->cdr = cdr;
}

list_t* pair(void* car, void* cdr) {
  list_t* c = malloc(sizeof(list_t));
  if (!c) return NULL;
  c->car = car;
  c->cdr = cdr;
  return c;
}

void cons_free(list_t* c, void* car_free, void* cdr_free) {
  if (NULL != car_free) ((void (*)(void*))car_free)(c->car);
  if (NULL != cdr_free) ((void (*)(void*))cdr_free)(c->cdr);
  free(c);
}

list_t *list_copy(list_t *lst, void *constructor) {
  assert(!constructor); // NYI
  list_t *head, **tail = &head;
  
  while (lst) {
    *tail = cons(lst->car, NULL);
    tail = &(*tail)->cdr;
    lst = lst->cdr;
  }
  return head;
}

unsigned int list_free(list_t* lst, void* destructor) {
  unsigned int num_removed = 0;
  list_t* next;
  while (lst) {
    num_removed++;
    if (NULL != destructor && lst->car) ((void (*) (void*))destructor)(lst->car);
    next = lst->cdr;
    free(lst);
    lst = next;
  }
  return num_removed;
}

void list_buffer_free(list_t *lst, void *destructor) {
  list_free(lst->cdr, destructor);  
  lst->car = NULL;
  lst->cdr = NULL;
}

void alist_free(list_t *lst, void *car_destructor, void *cdr_destructor) {
  assert(lst);
  list_t *init = lst;
  while (lst && lst->car) {    
    cons_free((list_t*)lst->car, car_destructor, cdr_destructor);
    lst = lst->cdr;
  }
  list_free(init, NULL);
}

// PREDICATES

int list_cmp(list_t* a, list_t* b, void* cmp) {
  if (a == b) return 0;
  int diff = 0;

  while (a && b) {
    diff = NULL == cmp ? 
      a->car != b->car : 
      ((int(*)(void*, void*))cmp)(a->car, b->car);
    if (diff) return diff;
    a = a->cdr;
    b = b->cdr;
  }
  return a || b;
}

int list_sorted(list_t *lst, int (*cmp)(void *, void *)) {
    list_t *prev = lst;
    lst = lst->cdr;
    while (lst) {
      if (0 < cmp(prev->car, lst->car)) return 0;
      prev = lst;
      lst = lst->cdr;
    }
    return 1;
}

int alist_cmp(list_t *a, list_t *b, void *cmp1, void *cmp2) {
  if (list_length(a) != list_length(b)) return -1;
  while (a && a->car) {
    if (compare(((list_t *)a->car)->car, 
		((list_t *)b->car)->car,
	cmp1) 
	|| compare(((list_t *)a->car)->cdr, 
		((list_t *)b->car)->cdr,
	cmp2)) return -1;
    a = a->cdr;
    b = b->cdr;
  }
  return 0;
}

// ACCESSORS

void* list_ref(list_t* lst, int n) {
  int i;
  for (i=0; i<n; ++i) {
    lst = lst->cdr;
  }
  return lst->car;
}

void* alist_ref(list_t *lst, void *idx, void* cmp) {
  while (lst && lst->car) {
    if (cmp ? !((int(*)(void*, void*))cmp)(idx, ((list_t*)lst->car)->car) : ((list_t*)lst->car)->car == idx) {
      return ((list_t*)lst->car)->cdr;
    }
    lst = lst->cdr;
  }
  return NULL;
}

list_t* list_tail(list_t *lst, int n) {
  while (n--) {
    if (!lst) return lst;
    lst = lst->cdr;
  }
  return lst;
}

list_t *list_tailf(list_t* lst, void* value, void* cmp) {
  while (lst && lst->car) {
    if (NULL == cmp ?
	lst->car == value :
	!((int(*)(void*, void*))cmp)(lst->car, value))
      return lst;
    lst = lst->cdr;
  }
  return NULL;
}

void* list_find(list_t* lst, void* value, void* cmp) {
  list_t *cell = list_tailf(lst, value, cmp);
  return cell ? cell->car : NULL;
}

list_t *list_findall(list_t *lst, void *value, void *cmp) {
  list_t *ret = NULL;
  while (lst && lst->car) {
    if (!compare(lst->car, value, cmp)) {
      ret = cons(lst->car, ret);
    }
    lst = lst->cdr;
  }
  return ret ? ret : list(0);
}

int list_index_of(list_t *lst, void* value, void *cmp) {
  int idx = -1;
  while (lst && lst->car) {
    idx++;
    if (cmp ? !((int(*)(void *, void *))cmp)(value, lst->car) : value == lst->car) {
      return idx;
    }
    lst = lst->cdr;
  }
  return -1;
}

int list_length(list_t* lst) {
  int length = 0;
  while (lst && lst->car) {
    ++length;
    lst = lst->cdr;
  }
  
  return length;
}


// MUTATORS

list_t *list_reverse(list_t *lst) {
  list_t *prev = NULL, *next;
  while (lst) {
    next = lst->cdr;
    lst->cdr = prev;
    prev = lst;
    lst = next;
  }
  return prev;
}

unsigned int list_clear(list_t* lst, void* destructor) {
  unsigned int num_removed = !!lst->car;
  if (NULL != destructor && lst->car) ((void (*) (void*))destructor)(lst->car);  
  num_removed += list_free(lst->cdr, destructor);
  lst->car = NULL;
  lst->cdr = NULL;
  return num_removed;
}

list_t *list_dropf(list_t* lst, void* value, void* cmp) {
  list_t *prev = NULL;
  while (lst && lst->car) {
    if (NULL == cmp ?
	lst->car == value :
	!((int(*)(void*, void*))cmp)(lst->car, value)) {
      if (prev) prev->cdr = NULL;
      else {
	list_t *tail = cons(lst->car, lst->cdr);
	lst->car = NULL;
	lst->cdr = NULL;
	return tail;
      }
      return lst;
    }
    prev = lst;
    lst = lst->cdr;
  }
  return NULL;
}

list_t *list_append(list_t *head, list_t *tail) {
  list_t *head_init = head;
  if (list_empty(head)) {
    free(head);
    return tail;
  }
  while (head->cdr) head = head->cdr;
  head->cdr = tail;
  return head_init;
}

void list_insert(list_t* lst, void* value, int pos) {
  assert(0 <= pos);

  if (list_empty(lst)) {
    lst->car = value;
  }
  else if (0 == pos) {
    lst->cdr = cons(lst->car, lst->cdr);
    lst->car = value;
  }
  else {
    assert(lst->car || lst->cdr);
    list_t *prev, *ins = list(1, value);
    while (lst && pos--) {
      prev = lst;
      lst = lst->cdr;
    }
    assert(prev); //TODO decide on list_insert failure mode
    ins->cdr = lst;
    prev->cdr = ins;
  }
}

void list_insert_unique(list_t *lst, void *value, void *cmp) {
  if (!list_find(lst, value, cmp)) list_insert(lst, value, 0);
}

//TODO NULL cmp should compare pointers
void* list_remove(list_t* lst, void* value, int (*cmp)(void*, void*)) {
  list_t* prev = lst;
  if (!lst->car) return NULL;
  while (lst) {
    assert(lst->car);
    //If we find a match,
    if (!cmp(lst->car, value)) {
      //extract the value and
      void* ret = lst->car; 
      //destroy the containing node.
      if (lst->cdr) {
        list_t* removed = lst->cdr;
        lst->car = removed->car;
        lst->cdr = removed->cdr;
        free(removed);
      }
      else {
        lst->car = NULL;
        prev->cdr = NULL;
        if (lst != prev) free(lst);
      }
      return ret;
    }
    prev = lst;
    lst = lst->cdr;
  }
  return NULL;
}


void* list_remove_sorted(list_t* lst, void* value, int (*cmp)(void*, void*)) {
  list_t* prev = lst;
  int cmp_val;
  if (!lst->car) return NULL;
  while (lst) {
    assert(lst->car);
    //If we find a match,
    cmp_val = cmp(lst->car, value);
    if (0 < cmp_val) return NULL;
    else if (!cmp_val) {
      //extract the value and
      void* ret = lst->car; 
      //destroy the containing node.
      if (lst->cdr) {
        list_t* removed = lst->cdr;
        lst->car = removed->car;
        lst->cdr = removed->cdr;
        free(removed);
      }
      else {
        lst->car = NULL;
        prev->cdr = NULL;
        if (lst != prev) free(lst);
      }
      return ret;
    }
    prev = lst;
    lst = lst->cdr;
  }
  return NULL;
}

void* list_remsert_sorted(list_t* lst, void* value, int (*cmp)(void*, void*), void *constructor) {
  list_t* prev = lst;
  int cmp_val;
  if (!lst->car) {
    lst->car = call_maybe(constructor, value);
    return NULL;
  }
  
  while (lst) {
    assert(lst->car);
    //If we find a match,
    cmp_val = cmp(lst->car, value);
    if (0 < cmp_val) {
      if (lst == prev) { // value is new head
	lst->cdr = cons(lst->car, lst->cdr);
	lst->car = call_maybe(constructor, value);
	return NULL;
      }
      else { // value belongs in mid list
	list_t *curr = cons(call_maybe(constructor, value), lst);
	prev->cdr = curr;
	return NULL;
      }
    }
    else if (!cmp_val) {
      //extract the value and
      void* ret = lst->car; 
      //destroy the containing node.
      if (lst->cdr) {
        list_t* removed = lst->cdr;
        lst->car = removed->car;
        lst->cdr = removed->cdr;
        free(removed);
      }
      else {
        lst->car = NULL;
        prev->cdr = NULL;
        if (lst != prev) free(lst);
      }
      return ret;
    }
    prev = lst;
    lst = lst->cdr;
  }
  prev->cdr = cons(call_maybe(constructor, value), NULL);
  return NULL;
}

list_t * list_taild(list_t *lst, int n, void *destructor) {
  list_t *prev;
  while (lst && n--) {
    if (!lst->cdr) {      
      lst->car = NULL;
      break;
    }
    prev = lst;
    lst = lst->cdr;
    if (destructor) ((void (*)(void*))destructor)(prev->car);
    free(prev);
  }
  return lst;
}

// HIGHER ORDER

void list_values(list_t *lst, ...) {
  int i, len = list_length(lst);  
  va_list args;
  va_start(args, lst);
  for (i=0; i<len; i++) {    
    void** ptr = va_arg(args, void**);
    *ptr = lst->car;
    lst = lst->cdr;
  }
  va_end(args);
}

/*
list_t *list_map(list_t *lst, void* (*f)(void*)) {
  list_t *head = NULL;
  list_t **tail = &head;
  while (lst && !list_empty(lst)) {
    *tail = list(1, f(lst->car));
    tail = &head->cdr;
    lst = lst->cdr;
  }
  return head;
}
*/

list_t *list_map(list_t *lst, void *f, list_t *args) {
  
  list_t *head = NULL;
  list_t **tail = &head;
  list_t app_args = {NULL, NULL};
  while (lst && !list_empty(lst)) {
    cons_init(&app_args, lst->car, args);
    *tail = cons(fn_apply(f, &app_args), NULL);
    tail = &head->cdr;
    lst = lst->cdr;
  }
  return head;
}

// THREADING

void list_buffer_write(volatile list_t *lst, void* value) {
  list_t *second = cons(value, lst->cdr);
  lst->cdr = second;
}

list_t * list_buffer_read(volatile list_t *lst) {
  assert(lst);
  while (!lst->cdr || !lst->cdr->cdr) sched_yield();

  list_t *head = lst->cdr;  
  list_t *tail = head->cdr;
  head->cdr = NULL;
  return tail;
}

list_t * list_buffer_readn_try(volatile list_t *lst, int n) {
  assert(lst && 0 <= n);
  if (!lst->cdr || !lst->cdr->cdr) return NULL;

  list_t *head = list_tail(lst->cdr, n);  
  list_t *tail = head->cdr;
  head->cdr = NULL;
  return tail;
}

list_t * list_buffer_read_try(volatile list_t *lst) {
  assert(lst);
  return list_buffer_readn_try(lst, 0);
}



// SORTING


void list_merge_into(list_t* lst, list_t* ins, int (*cmp)(void*, void*)) {
  void *swap;
  list_t *next = lst->cdr;

  // Empty insertions can be thrown out.
  if (list_empty(ins)) free(ins);
  //If the list is empty, insert the value.
  else if (!lst->car) {
    lst->car = ins->car;
    lst->cdr = ins->cdr;
    free(ins);
  }
  // If the first element is an ins point, overwrite it.
  else {
    if (0 <= cmp(lst->car, ins->car)) {
      swap = lst->car;
      lst->car = ins->car;
      ins->car = swap;
      next = ins->cdr;
      ins->cdr = lst->cdr;    
    }
    lst->cdr = list_merge(ins, next, cmp);
  }  
}

list_t *list_merge(list_t *head, list_t *middle, int (*cmp)(void*, void*)) {
  list_t* sorted = NULL;
  list_t** next = &sorted;
  while (head && middle) {
    if (cmp(head->car, middle->car) < 0) {
      *next = head;
      next = &head->cdr;
      head = head->cdr;      
    }
    else {
      *next = middle;
      next = &middle->cdr;
      middle = middle->cdr;      
    }
  }

  if (head) *next = head;
  else *next = middle;
  return sorted;
}

list_t* list_mergesort(list_t* head, int (*cmp)(void*, void*)) {
  if (!head || !head->cdr) return head;

  list_t* slow, *fast;
  slow = fast = head;
  while (fast->cdr && fast->cdr->cdr) {
    slow = slow->cdr;
    fast = fast->cdr->cdr;
  }
  list_t* middle = slow->cdr;
  slow->cdr = NULL;

  head = list_mergesort(head, cmp);
  middle = list_mergesort(middle, cmp);

  return list_merge(head, middle, cmp);
}

// COERSION

void** list2array(list_t* lst) {
  int i = 0;

  void** ary = malloc(sizeof(void*) * list_length(lst));
  if (!ary) return NULL;
  while (lst) {
    ary[i] = lst->car;
    lst = lst->cdr;
    ++i;
  }
  return ary;
}

// I/O

void list_print(list_t* lst, void(*printer)(void*)) {
  printf("(");
  while (lst) {
    printer(lst->car);
    lst = lst->cdr;
    if (lst) printf(" ");
  }
  printf(")");
}

void alist_print(list_t* lst, void(*p1)(void*), void(*p2)(void*)) {
  printf("(");
  while (lst && lst->car) {
    printf("(");
    p1(((list_t *)lst->car)->car);
    printf(" ");
    p2(((list_t *)lst->car)->cdr);
    printf(")");
    lst = lst->cdr;
    if (lst) printf(" ");
  }
  printf(")");
}
