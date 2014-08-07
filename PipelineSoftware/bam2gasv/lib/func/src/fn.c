#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include "fn.h"
#include "list.h"

void* fn_call(void* f, int n, ...) {
  va_list args;  
  va_start(args, n);
  void *ret = fn_vcall(f, n, args);
  va_end(args);
  return ret;  
}

void* fn_vcall(void* f, int n, va_list args) {
  
  assert(n <= 3);
  void *a0, *a1, *a2;
  switch (n) {
  case 0:
    return ((void*(*)())f)();
  case 1:
    return ((void*(*)(void*))f)(va_arg(args, void*));
  case 2:
    a0 = va_arg(args, void*);
    a1 = va_arg(args, void*);
    return ((void*(*)(void*, void*))f)(a0, a1);
  case 3:
    a0 = va_arg(args, void*);
    a1 = va_arg(args, void*);
    a2 = va_arg(args, void*);
    return ((void*(*)(void*, void*, void*))f)(a0, a1, a2);
  }

  return NULL;
}

void* fn_apply(void* f, list_t *args) {
  void *a0, *a1, *a2;
  list_values(args, &a0, &a1, &a2);
  return fn_call(f, list_length(args), a0, a1, a2);
}

void* fn_applyd(void* f, list_t *args) {
  void *ret = fn_apply(f, args);
  list_free(args, NULL);
  return ret;
}
