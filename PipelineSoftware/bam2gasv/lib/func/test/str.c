#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "str.h"


int main() {
  
  char *foo = "foo";
  char *bar = str_copy(foo);
  assert(!strcmp(foo, bar));
  free(bar);


  return 0;
}
