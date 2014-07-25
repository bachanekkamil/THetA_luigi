// If a LARGE_BAM has been split into smaller bams, this component merges each output of the smaller BAMs into a single output representing the original BAM file. This component uses information about the number of files and libraries to generate the file paths where all thee split BAMS will reside, and then starts two processes: one to read each BAM into a series of buffers and one to merge those buffers into the final files for each (library x .variant_type) pair.

#ifndef PAIR_FILES2BTG_OUTPUT_H
#define PAIR_FILES2BTG_OUTPUT_H

#include "list.h"
#include "b2g_chromosome.h"
#include "b2g_libraries.h"

void pair_files2gasv_input(char *, int , b2g_libraries_t *, b2g_chromosomes_t *, int, int, int);

#endif
