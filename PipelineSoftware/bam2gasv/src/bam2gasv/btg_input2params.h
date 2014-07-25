
// Parses command line arguments into B2G parameters.

#ifndef B2G_BTG_INPUT2PARAMS_H
#define B2G_BTG_INPUT2PARAMS_H

#include "b2g_error.h"
#include "b2g_cutoff_lminlmax.h"
#include "b2g_constants.h"

// Parses main's argc and argv into the program's "global" variables. Returns a b2g_error_t describing the success or specific failing command of the parsing.
b2g_error_t btg_input2params(int, char**, char**, char**, int*, int*, int*,  int*, int*, int *, int *, int *, int *, b2g_cutoff_lminlmax_mode_t *, char **, int *, int *, int *, int *, int *, char **, b2g_platform_t *, int *, int *, int *, int *, int *);

#endif
