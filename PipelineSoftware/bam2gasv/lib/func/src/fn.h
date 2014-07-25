#ifndef FUNC_FN_H
#define FUNC_FN_H

#include <stdarg.h>
#include "list.h"

void* fn_call(void*, int, ...);
void* fn_vcall(void*, int, va_list);
void* fn_apply(void*, list_t *);
void* fn_applyd(void *, list_t *);

#endif
