// This is the highest level function that runs the entire program logic. This function acquires all filesystem resources required to run the program (aborting with an appropriate error on failure), splits the bam file into smaller bam files as needed for large bams (params2bam_files), pairs and sorts each bam file (bam_files2pair_files), and merges the results together for the final output (pair_files2gasv_input).

#ifndef B2G_PARAMS2GASV_INPUT_H
#define B2G_PARAMS2GASV_INPUT_H

#include "b2g_cutoff_lminlmax.h"
#include "b2g_constants.h"
#include "b2g_error.h"

b2g_error_t params2gasv_input(char *, char *, int , int , int , int , int , int , int , int , int , b2g_cutoff_lminlmax_mode_t , char *, int , int, int, int, int, char *, b2g_platform_t, int, int, int, int, int, unsigned int *);

#endif
