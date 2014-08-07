#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include "hash.h"

int intcmp(void* a, void* b) {
  return *(int*)a - *(int*)b;
}

void int_print(void* i) {
  printf("%d", *(int*)i);
}

int main() {

  int nums[] = {0, 1, 2, 3, 4};
  hash_t* hsh;
  int* num;
  
  hsh = hash(10);  
  num = malloc(sizeof(int));
  hash_remsert(hsh, num, 0, intcmp, NULL);
  hash_free(hsh, free);

  
  hsh = hash(2);
  assert(NULL == hash_remsert(hsh, &nums[0], nums[0], intcmp, NULL));
  assert(NULL == hash_remsert(hsh, &nums[1], nums[1], intcmp, NULL));
  assert(NULL == hash_remsert(hsh, &nums[2], nums[2], intcmp, NULL));
  assert(&nums[1] == hash_remsert(hsh, &nums[1], nums[1], intcmp, NULL));    
  hash_free(hsh, NULL);

  hsh = hash(2);
  hash_insert(hsh, &nums[2], nums[2], intcmp, NULL);
  hash_insert(hsh, &nums[2], nums[2], intcmp, NULL);
  list_t *b = hash_bucket(hsh, nums[2]);
  assert(&nums[2] == list_ref(b, 0));
  assert(&nums[2] == list_ref(b, 1));
  hash_free(hsh, NULL);

  assert(6953516687550 == djb2_hash("foobar"));

  return 0;
}
