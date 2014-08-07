#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "io.h"


FILE* file_open(char* path, char* mode, ...) {
  va_list args;
  va_start(args, mode);
  char filename[2048];
  vsprintf(filename, path, args);
  va_end(args);
  return fopen(filename, mode);
}

void print_str(void *str) {
  printf("%s", (char *)str);
}

void print_int(void *i) {
  printf("%d", *((int *)i));
}

void nl() {
  printf("\n");
}

/*
//TODO handle errors
void* file2bytes(FILE* file) {
  char buffer[1000];
  size_t nbytes = sizeof(buffer);
  ssize_t bytes_read;
  unsigned int total_bytes = 0;

  while ((bytes_read = read(file nbytes, buffer, nbytes))) {
    total_bytes += bytes_read;
  }
  
}
*/
