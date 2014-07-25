// This component consumes paired reads and produces the same pairs in sorted order. This function spawns a separate thread to concurrently sort each variant_type.
// Input is a list_t [file_id][library][variant_type] where the tail of each list_t contains a series of b2g_pair_t'sF. Each list_t is being concurrently prepended with new bams, so this process will chop off the tail, sort the chopped tail, add the tail to a growing list of sorted pairs, and chop off another tail. When we see that the list_t begins with the EOF, we know that this tail represents the last series of bams in a given BAM file and we may flush our accumulation of sorted pairs to the next component and begin a new accumulation on the next file. The accumulating list is merged with the sorted tail at each step as though part of a substep of a merge sort.
// Output is a buffer_t *[variant_type] where each buffer contains a stream of each library for each file in turn (ie file1_lib1, file1_lib2 ...).

#ifndef UNSORTED_PAIRS2PAIRS_H
#define UNSORTED_PAIRS2PAIRS_H

#include <pthread.h>
#include "buffer.h"
#include "b2g_signal.h"
#include "b2g_variant_type.h"
#include "list.h"

void unsorted_pairs2pairs_init(pthread_t [NUM_VTYPES],int *, list_t **, buffer_t *[NUM_VTYPES], int *, int, int *, int *);


#endif
