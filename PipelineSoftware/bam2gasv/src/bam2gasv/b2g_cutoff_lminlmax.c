#include <assert.h>
#include <stdio.h>
#include "b2g_cutoff_lminlmax.h"

int b2g_cutoff_lminlmax_read(FILE *in, b2g_cutoff_lminlmax_mode_t *mode, int *X, int *Y, char *name) {
  assert(in && mode && X && Y && name);

  if (-1 == fscanf(in, "%s\t", name)) return -1; // Parse the library name,
  // then the parameters.
  else if (1 == fscanf(in, "SD=%d", X)) *mode = SD;
  else if (1 == fscanf(in, "PCT=%d%%", X)) *mode = PCT;
  else if (2 == fscanf(in, "EXACT=%d,%d", X, Y)) *mode = EXACT;
  else return -2;
  
  return 0;
}
