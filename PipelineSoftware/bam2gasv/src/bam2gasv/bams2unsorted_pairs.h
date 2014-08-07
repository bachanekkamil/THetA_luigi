// Starts a thread that consumes reads and produces pairs. 
//The input buffer contains a serios of bam1_t's, punctuated by B2G_SIGNAL_EOF to mark the end of each split BAM file (or just the single BAM file if !LARGE_BAM). The output buffers are:
//lowq_pairs, which accepts a sequence of bam1_t's and a final B2G_SIGNAL_EOF
// file2lib2vtype2unsorted_pairs, which is an array of list_t's, each representing pairs belonging to a specific BAM file within a given library and variant type. Each list_t will contain pairs in its tail, with the head being reserved for a B2G_SIGNAL_EOF to signal that no more pairs will be prepended to the tail.

#ifndef B2G_BAMS2UNSORTED_PAIRS_H
#define B2G_BAMS2UNSORTED_PAIRS_H

#include <pthread.h>
#include "buffer.h"
#include "b2g_libraries.h"
#include "hash.h"

void bams2unsorted_pairs_init(pthread_t *,buffer_t *, buffer_t *, list_t *, b2g_libraries_t *, b2g_chromosomes_t *, hash_t *, int *, int *, int *, int *, int *, int *, int *, int *, b2g_platform_t *, int *, int *, int *, int *, int *, int *, int *, unsigned int *);

#endif
