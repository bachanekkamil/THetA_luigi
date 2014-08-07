// Provides a datastructure representing -CUTOFF_LMINLMAX data and a parser for reading it from a file.

#ifndef B2G_CUTOFF_LMINLMAX_H
#define B2G_CUTOFF_LMINLMAX_H

#include <stdio.h>


// DATATYPES

typedef enum {
  NO_MODE, PCT, SD, EXACT, FILENAME
} b2g_cutoff_lminlmax_mode_t;

typedef struct {
  b2g_cutoff_lminlmax_mode_t mode;
  int X; // lmin, number of standard deviations, or percent quantile, respectively depending on mode
  int Y; // lmax in EXACT mode
  char *name; // file name in FILE mode
} b2g_cutoff_lminlmax_t;


// SERIALIZATION


// Takes a file of <libname>\t<CUTOFF_LMINLMAX> pairs and reads the next line into mode, X, Y and name.
// Returns a 0 if there are more lines, -1 when done, or -2 if an incorrectly formatted line is encountered.
int b2g_cutoff_lminlmax_read(FILE *, b2g_cutoff_lminlmax_mode_t *, int *, int *, char *);

#endif
