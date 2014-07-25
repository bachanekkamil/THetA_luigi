// This function processes all BAM files and outputs the corresponding gasv files for later merging. It uses the number of files to generate all bam filenames, and pipes them in sequence through a series of worker threads. If we are not using LARGE_BAM then this will take a BAM file directly to its final output (with appropriate naming conventions).

#ifndef B2G_BAM_FILES2_GASV_FILES_H
#define B2G_BAM_FILES2_GASV_FILES_H

#include "b2g_cutoff_lminlmax.h"
#include "b2g_libraries.h"
#include "sam.h"
#include "b2g_chromosome.h"
#include "hash.h"

b2g_error_t bam_files2pair_files(int , char *, char *, b2g_libraries_t *, b2g_chromosomes_t *, hash_t *, samfile_t *, samfile_t *, int, int, int, int, int, int, int, int, int, int, int, b2g_platform_t, int, int, int, int, unsigned int *);

#endif
