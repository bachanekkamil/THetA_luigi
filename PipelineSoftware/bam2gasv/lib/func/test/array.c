#include <stdlib.h>
#include <assert.h>
#include "array.h"

int main() {

  int n[] = {0, 1, 2, 3, 4};

  array_t *a = array(3, sizeof(int));
  assert(!*((int*)array_get(a, 1)));
  array_set(a, 1, &n[1]);
  assert(1 == *((int*)array_get(a, 1)));
  array_free(a, NULL);

  return 0;
}

