#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "cons.h"


int main() {
  
  // CONSTRUCTOR
  void* car = malloc(sizeof(int));;
  int cdr = 2;
  cons_t* c = cons(car, &cdr);
  assert(c->car == car);
  assert(c->cdr == &cdr);
  cons_free(c, free, NULL);

  return 0;
}
