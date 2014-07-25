#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "str.h"

char *str_copy(char *str) {
  char *cpy = malloc(sizeof(char) * (strlen(str) + 1));
  strcpy(cpy, str);
  assert(!strcmp(str, cpy));
  return cpy;
}

void str_print(char *str) {
  printf("%s", str);
}
