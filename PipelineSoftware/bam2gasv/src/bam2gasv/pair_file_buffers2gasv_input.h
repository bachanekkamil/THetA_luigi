// Consumes pairs from a number of sorted buffers and merges them into one output file per (library x variant_type) pair. This component consumes a buffer *[library][variant_type][split_file_id] and for each (library x variant_type) pair, constructs a min heap of buffers sorted by the sort order of the first (lowest, since buffers are sorted) pair in a buffer. The component then extracts the minimum from the heap until all buffers for all split files for that given (library x variant_type) pair. The protocol for the array of buffers requires that each buffer contain a sequence of disc or conc pairs followed by a single B2G_SIGNAL_EOF to mark the end of the split file.

#ifndef B2G_PAIR_FILE_BUFFERS2GASV_INPUT_H
#define B2G_PAIR_FILE_BUFFERS2GASV_INPUT_H

#include "buffer.h"
#include "b2g_variant_type.h"
#include "list.h"
#include "b2g_chromosome.h"
#include "b2g_libraries.h"

void pair_file_buffers2gasv_input_init(pthread_t *, buffer_t *[], char *, int *, b2g_chromosomes_t *, b2g_libraries_t *, int *, int *, int *);

#endif
