#include <stdlib.h>
#include <stdio.h>
#include "integer.h"

int *integer(int n) {
  int *num = malloc(sizeof(int));
  *num = n;
  return num;
}

void integer_print(int *n) {
  printf("%d", *n);
}
