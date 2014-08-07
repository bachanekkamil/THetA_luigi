// Consumes lists of sorted pairs, each representing the contets of a BAM file, and writes them to disk in a format that can be read back in later to merge them into the final output. Starts one thread per variant_type pair, which sleeps until a list is available.
// Input is an array of buffers that contain, for each BAM file, its libraries in order in the stream (ie file1_lib1, file1_lib2 ...).

#ifndef B2G_PAIRS2PAIR_FILES_H
#define B2G_PAIRS2PAIR_FILES_H

#include <stdio.h>
#include <pthread.h>
#include "buffer.h"
#include "list.h"
#include "b2g_variant_type.h"
#include "b2g_libraries.h"


void pairs2pair_files_init(pthread_t [NUM_VTYPES], buffer_t *[NUM_VTYPES], char *, int, int, int *, b2g_libraries_t *, b2g_chromosomes_t *, int *, int *, int *, int *);

#endif
