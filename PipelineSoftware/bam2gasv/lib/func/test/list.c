#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

int intcmp(void* a, void* b) {
  if (!a || !b) return 0;
  return *(int*)a - *(int*)b;
}

void int_print(void* i) {
  if (i) printf("%d", *(int*)i);
}

void* int_shift(void* p) {
  return p + sizeof(int);
}

void *int_add1(void *i) {
  void *ret = malloc(sizeof(int));
  *((int*)ret) = *((int*)i) + 1;
  return ret;
}

void *int_3add(void *a0, void *a1, void *a2) {
  void *ret = malloc(sizeof(int));
  *((int*)ret) = *((int*)a0) + *((int*)a1) + *((int*)a2);
  return ret;
}

int int_lt(int *a, int *b) {
  return *b < *a;
}

int check_list_cmp(list_t *a, list_t *b) {
  
  int pass = list_cmp(a, b, NULL);
  list_free(b, NULL);
  return !pass;
}


int main() {

  int n[] = {0, 1, 2, 3, 4};
  int *zero = &n[0];
  int *one = &n[1];
  int *two = &n[2];
  int *three = &n[3];
  int *four = &n[4];


  list_t *lst, *lst2, *lst3;
  int *n1 = NULL, *n2 = NULL;

  lst = list(0);
  assert(NULL == lst->car);
  assert(NULL == lst->cdr);
  list_free(lst, NULL);

  //stress test list_free
  lst = list(2, NULL, malloc(sizeof(char)));
  list_free(lst, free);

  //test list_clear
  lst = list(2, malloc(sizeof(char)),
	     malloc(sizeof(char)));
  list_clear(lst, free);
  assert(NULL == lst->car);
  assert(NULL == lst->cdr);
  free(lst);


  //test list_ref
  lst = list(3, &n[0], &n[1], &n[4]);
  assert(&n[4] == list_ref(lst, 2));
  list_free(lst, NULL);

  //test alist_ref
  lst = list(2, pair(&n[0], &n[1]), 
	     pair(&n[2], &n[3]));
  lst2 = list(1, pair(malloc(sizeof(int)), malloc(sizeof(int))));
  lst3 = list(0);
  assert(NULL == alist_ref(lst3, &n[0], NULL));
  assert(&n[1] == alist_ref(lst, &n[0], NULL));
  assert(&n[1] == alist_ref(lst, 0, intcmp));
  assert(!alist_cmp(lst, lst, intcmp, intcmp));
  assert(alist_cmp(lst, lst3, intcmp, intcmp));
  
  alist_free(lst, NULL, NULL);
  alist_free(lst2, free, free);  
  list_free(lst3, NULL);

  //test list_tail
  lst = list(3, &n[0], &n[1], &n[4]);
  lst2 = list(2, &n[1], &n[4]);
  assert(!list_cmp(list_tail(lst, 1), lst2, NULL));
  assert(NULL == list_tail(lst, 4));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  // test list_copy
  lst = list(3, zero, one, two);
  assert(check_list_cmp(lst, list_copy(lst, NULL)));
  list_free(lst, NULL);

  // test list_sorted
  lst = list(3, zero, one, two);
  assert(list_sorted(lst, intcmp));
  list_free(lst, NULL);
  lst = list(3, zero, two, one);
  assert(!list_sorted(lst, intcmp));
  list_free(lst, NULL);

  //test list_dropf
  lst = list(3, &n[0], &n[1], &n[4]);
  lst2 = list(2, &n[1], &n[4]);
  lst3 = list_dropf(lst, &n[1], intcmp);
  assert(!list_cmp(lst2, lst3, NULL));
  list_free(lst2, NULL);
  list_free(lst3, NULL);
  lst2 = list(1, &n[0]);
  assert(!list_cmp(lst2, lst, NULL));
  assert(!list_dropf(lst, &n[1], intcmp));  
  lst3 = list_dropf(lst, &n[0], intcmp);
  assert(list_empty(lst));
  assert(!list_cmp(lst3, lst2, NULL));
  list_free(lst2, NULL);
  list_free(lst3, NULL);
  list_free(lst, NULL);
  
  lst = list(4, &n[0], &n[1], &n[1], &n[4]);
  assert(!list_dropf(lst, &n[3], NULL));
  lst2 = list(4, &n[0], &n[1], &n[1], &n[4]);
  lst3 = list_dropf(lst, &n[0], NULL);
  assert(!list_cmp(lst2, lst3, NULL));
  list_free(lst2, NULL);
  list_free(lst3, NULL);
  list_free(lst, NULL);


  //test list_buffer_read and list_buffer_write
  lst = list(3, &n[0], &n[1], &n[4]);
  lst2 = list(1, &n[4]);
  lst3 = list_buffer_read(lst);
  assert(!list_cmp(lst3, lst2, NULL));
  lst2->car = &n[1];
  list_buffer_write(lst, &n[2]);
  list_free(lst3, NULL);
  lst3 = list_buffer_read(lst);
  assert(!list_cmp(lst3, lst2, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  list_free(lst3, NULL);  

  //test list_find
  lst = list(3, &n[0], &n[1], &n[2]);
  assert(&n[1] == list_find(lst, &n[1], NULL));
  list_free(lst, NULL);

  //test list_filter
  lst = list(3, &n[2], &n[1], &n[2]);
  lst3 = list_findall(lst, &n[2], NULL);
  lst2 = list(2, &n[2], &n[2]);
  assert(!list_cmp(lst3, lst2, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  list_free(lst3, NULL);

  // test list_index_of
  lst = list(3, &n[0], &n[1], &n[2]);
  assert(1 == list_index_of(lst, &n[1], intcmp));
  assert(2 == list_index_of(lst, &n[2], NULL));
  assert(-1 == list_index_of(lst, &n[3], NULL));
  list_free(lst, NULL);

  // test list_cmp
  lst = list(2, &n[0], &n[1]);
  lst2 = list(2, &n[0], &n[2]);
  assert(0 == list_cmp(lst, lst, NULL));
  assert(1 == list_cmp(lst, lst2, NULL));
  assert(0 == list_cmp(lst, lst, intcmp));
  assert(0 != list_cmp(lst, lst2, intcmp));
  list_free(lst, NULL);
  list_free(lst2, NULL);




  //test list_empty
  lst = list(0);
  lst2 = list(1, &n[0]);
  assert(list_empty(lst));
  assert(!list_empty(lst2));
  list_free(lst, NULL);
  list_free(lst2, NULL);


  //test list_insert
  lst = list(2, &n[0], &n[2]);
  lst2 = list(5, &n[0], &n[0], &n[1], &n[2], &n[3]);
  list_insert(lst, &n[1], 1);
  list_insert(lst, &n[3], 3);
  list_insert(lst, &n[0], 0);
  assert(!list_cmp(lst, lst2, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  //list_insert_unique
  lst = list(0);
  lst2 = list(1, &n[0]);
  lst3 = list(2, &n[1], &n[0]);
  list_insert_unique(lst, &n[0], intcmp);
  assert(!list_cmp(lst, lst2, NULL));
  list_insert_unique(lst, &n[0], NULL);
  assert(!list_cmp(lst, lst2, NULL));
  list_insert_unique(lst, &n[1], NULL);
  assert(!list_cmp(lst, lst3, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  list_free(lst3, NULL);

  //test list_taild
  lst = list(3, &n[0], &n[1], &n[2]);
  lst2 = list(2, &n[1], &n[2]);
  lst3 = list(0);
  assert(!list_cmp(lst, list_taild(lst, 0, NULL), NULL));  
  assert(!list_cmp(lst2, (lst = list_taild(lst, 1, NULL)), NULL));
  assert(!list_cmp(lst3, (lst = list_taild(lst, 2, NULL)), NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  list_free(lst3, NULL);


  //test list_remove
  lst = list(3, &n[0], &n[1], &n[2]);
  list_remove(lst, &n[1], intcmp);
  list_remove(lst, &n[1], intcmp);
  lst2 = list(2, &n[0], &n[2]);
  assert(0 == list_cmp(lst, lst2, intcmp));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  //test list_remove_sorted
  lst = list(3, &n[0], &n[1], &n[2]);
  list_remove_sorted(lst, &n[1], intcmp);
  list_remove_sorted(lst, &n[1], intcmp);
  lst2 = list(2, &n[0], &n[2]);
  assert(0 == list_cmp(lst, lst2, intcmp));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  //test list_remove_sorted
  lst = list(0);
  assert(!list_remsert_sorted(lst, one, intcmp, NULL));
  assert(check_list_cmp(lst, list(1, one)));
  assert(!list_remsert_sorted(lst, zero, intcmp, NULL));
  assert(check_list_cmp(lst, list(2, zero, one)));
  assert(!list_remsert_sorted(lst, two, intcmp, NULL));
  assert(check_list_cmp(lst, list(3, zero, one, two)));
  assert(one == list_remsert_sorted(lst, one, intcmp, NULL));
  assert(check_list_cmp(lst, list(2, zero, two)));

  assert(!list_remsert_sorted(lst, three, intcmp, NULL));
  assert(check_list_cmp(lst, list(3, zero, two, three)));
  
  assert(!list_remsert_sorted(lst, four, intcmp, NULL));
  assert(check_list_cmp(lst, list(4, zero, two, three, four)));
  
  assert(zero == list_remsert_sorted(lst, zero, intcmp, NULL));
  assert(check_list_cmp(lst, list(3, two, three, four)));

  assert(!list_remsert_sorted(lst, zero, intcmp, NULL));
  assert(check_list_cmp(lst, list(4, zero, two, three, four)));

  assert(three == list_remsert_sorted(lst, three, intcmp, NULL));
  assert(check_list_cmp(lst, list(3, zero, two, four)));
  
  assert(four == list_remsert_sorted(lst, four, intcmp, NULL));
  assert(check_list_cmp(lst, list(2, zero, two)));

  assert(check_list_cmp(lst, list(2, zero, two)));
  assert(two == list_remsert_sorted(lst, two, intcmp, NULL));
  assert(check_list_cmp(lst, list(1, zero)));

  assert(!list_remsert_sorted(lst, one, intcmp, NULL));
  assert(check_list_cmp(lst, list(2, zero, one)));
  assert(one == list_remsert_sorted(lst, one, intcmp, NULL));
  assert(check_list_cmp(lst, list(1, zero)));
  

  assert(zero == list_remsert_sorted(lst, zero, intcmp, NULL));
  assert(list_empty(lst));
  list_free(lst, NULL);

  //test list_insert_sorted
  lst = list(2, &n[0], &n[2]);
  lst2 = list(2, &n[1], &n[3]);
  list_merge_into(lst2, lst, intcmp);
  list_merge_into(lst2, list(0), intcmp);
  lst3 = list(4, &n[0], &n[1], &n[2], &n[3]);
  assert(0 == list_cmp(lst2, lst3, NULL));
  list_free(lst2, NULL); //since lst merged into lst2, only free lst2
  list_free(lst3, NULL);
  lst = list(0);
  lst2 = list(2, &n[1], &n[3]);
  lst3 = list(2, &n[1], &n[3]);
  list_merge_into(lst, lst2, NULL);
  assert(!list_cmp(lst, lst3, NULL));
  list_free(lst, NULL);
  list_free(lst3, NULL);


  //test list_mergesort
  lst = list(5, &n[4], &n[3], &n[2], &n[0], &n[1]);
  lst2 = list(5, &n[0], &n[1], &n[2], &n[3], &n[4]);
  lst = list_mergesort(lst, intcmp);  
  assert(!list_cmp(lst, lst2, intcmp));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  //test list_length
  lst = list(0);
  lst2 = list(1, &n[0]);
  lst3 = list(2, &n[0], &n[0]);
  assert(0 == list_length(lst));
  assert(1 == list_length(lst2));
  assert(2 == list_length(lst3));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  list_free(lst3, NULL);

  //test list_map
  lst = list(2, &n[0], &n[1]);
  lst2 = list_map(lst, int_add1, NULL);
  lst3 = list(2, &n[1], &n[2]);
  assert(!list_cmp(lst2, lst3, intcmp));
  list_free(lst2, free);
  lst2 = list_map(lst, int_3add, lst3);
  list_free(lst, NULL);
  lst = list(2, &n[3], &n[4]);
  assert(!list_cmp(lst2, lst, intcmp));
  list_free(lst, NULL);
  list_free(lst2, free);
  list_free(lst3, NULL);

  //test list_values
  assert(!n1 && !n2);
  lst = list(2, &n[1], &n[2]);
  list_values(lst, &n1, &n2);
  assert(n1 == &n[1] && n2 == &n[2]);
  list_free(lst, NULL);

  // test list_append
  lst = list(1, &n[0]);
  lst2 = list(1, &n[1]);
  lst3 = list(2, &n[0], &n[1]);
  assert(!list_cmp(list_append(lst, lst2), lst3, NULL));
  list_free(lst, NULL);
  list_free(lst3, NULL);

  // test list_reverse
  lst = list(3, &n[0], &n[1], &n[2]);
  lst = list_reverse(lst);
  lst2 = list(3, &n[2], &n[1], &n[0]);
  assert(!list_cmp(lst, lst2, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);
  lst = list(1, &n[1]);
  lst = list_reverse(lst);
  lst2 = list(1, &n[1]);
  assert(!list_cmp(lst, lst2, NULL));
  lst->car = NULL;
  lst2->car = NULL;
  lst = list_reverse(lst);
  assert(!list_cmp(lst, lst2, NULL));
  list_free(lst, NULL);
  list_free(lst2, NULL);

  return 0;
}
