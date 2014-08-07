#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "fn.h"
#include "list.h"

void* id0() {
  return NULL;
}

void* id1(void *v1) {
  return v1;
}

void* id2(void *v1, void *v2) {
  return v2;
}

void* id3(void *v1, void *v2, void *v3) {
  return v3;
}

int main() {

  list_t *lst;
  int n[] = {0, 1, 2, 3, 4};

  //test fn_call
  assert(NULL == fn_call(id0, 0));
  assert(&n[0] == fn_call(id1, 1, &n[0]));
  assert(&n[1] == fn_call(id2, 2, &n[0], &n[1]));
  assert(&n[2] == fn_call(id3, 3, &n[0], &n[1], &n[2]));

  lst = list(2, &n[0], &n[1]);
  assert(&n[1] == fn_apply(id2, lst));
  list_free(lst, NULL);

  return 0;
}
