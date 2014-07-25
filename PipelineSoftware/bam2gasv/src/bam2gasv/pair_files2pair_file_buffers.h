// Spawns a thread that consumes files of pairs and loads them into buffers to be merged and written out to the final output. This takes a buffer *[library][variant_type][split_file_id] and for each (library x variant_type) pair, it iterates over all split_file_ids, loading as many pairs as will fit into the corresponding buffer. The thread will keep iterating over those split_file_ids until enough pairs have been read out of these buffers by another thread to load the entirety of data for the (library x variant_type) pair into the buffers. The protocol for the array of buffers requires each buffer to contain b2g_disc_pair_t or b2g_conc_pair_t's as appropriate, followed by a single B2G_SIGNAL_EOF to signal the end of a given split file.

#ifndef B2G_PAIR_FILES2PAIR_FILE_BUFFERS_H
#define B2G_PAIR_FILES2PAIR_FILE_BUFFERS_H

#include "list.h"
#include "b2g_variant_type.h"
#include "buffer.h"
#include "b2g_libraries.h"

void pair_files2pair_file_buffers_init(pthread_t *, buffer_t **, char * , int *, b2g_libraries_t *, int *, int *);

#endif
