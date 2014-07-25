// Constructs a thread that consumes pairs from a buffer and writes them to a lowq or splitread file as appropriate.
// The buffer should consist of a stream of bam1_t's with a final B2G_SIGNAL_EOF

#ifndef UNSORTED_PAIRS2LOWQ_FILE_H
#define UNSORTED_PAIRS2LOWQ_FILE_H

#include <pthread.h>
#include "buffer.h"
#include "sam.h"

void unsorted_pairs2lowq_file_init(pthread_t *, buffer_t * , samfile_t **, samfile_t **, int *, int *, int *, int *, int *);

#endif
