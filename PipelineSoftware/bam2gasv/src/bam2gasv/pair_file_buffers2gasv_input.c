#include <stdlib.h>
#include <pthread.h>
#include "pair_file_buffers2gasv_input.h"
#include "b2g_variant_type.h"
#include "buffer.h"
#include "heap.h"
#include "b2g_signal.h"
#include "b2g_pair.h"
#include "io.h"
#include "list.h"
#include "integer.h"
#include "b2g_chromosome.h"
#include "b2g_libraries.h"
#include "b2g_pair_file.h"

// Compares the first pair of each buffer. EOF/EOAF signals float to the bottom of the heap so that we encounter all the data before we see an end signal.
static int gasv_minheap_cmp(void *a_buff, void *b_buff) {
  b2g_pair_t *a = buffer_peek((buffer_t *)a_buff);
  b2g_pair_t *b = buffer_peek((buffer_t *)b_buff);

  if (b2g_signal_eoaf(a)) return 1;
  if (b2g_signal_eoaf(b)) return -1;

  if (b2g_signal_eof(a)) return 1;
  if (b2g_signal_eof(b)) return -1;
  
  return b2g_pair_cmp(a, b);
}


// Given a particular (library x variant_type) pair, merge all split BAM files together in sort order. Uses a min-heap sort strategy which can handle the changing value of the advancing file buffers.
static void _merge_files(buffer_t **fid2sorted_pairs, int num_files, b2g_chromosomes_t *chromosomes, b2g_variant_t vtype, char *OUTPUT_PREFIX, b2g_libraries_t *libraries, int lib, int GASV_PRO, int SPLIT_BY_CHROMOSOME) {

  int j, lchr = -1, rchr = -1, prev_lchr = -1, prev_rchr = -1;
  heap_t *pair_minheap = heap(num_files);
  FILE *out = NULL;

  // Insert all (sorted) buffers into a min heap based on the first element in the buffer.
  for (j=0; j<num_files; j++) {
    heap_insert(pair_minheap, *(fid2sorted_pairs + j), gasv_minheap_cmp);
  }

  // While there are non-empty buffers in the heap.
  while (!b2g_signal_eof(buffer_peek(heap_peek(pair_minheap)))) {
    assert(!b2g_signalp(buffer_peek(heap_peek(pair_minheap))));
     
    // make sure we have the matching chr file,
    lchr = b2g_pair_lchr((b2g_pair_t *)buffer_peek(heap_peek(pair_minheap)));
    rchr = b2g_pair_rchr((b2g_pair_t *)buffer_peek(heap_peek(pair_minheap)));
    if (lchr != prev_lchr || rchr != prev_rchr) {
      if (out) fclose(out);
      out = _open_gasv_input_file(OUTPUT_PREFIX, libraries, lib, vtype, GASV_PRO, SPLIT_BY_CHROMOSOME, lchr, rchr);
      prev_lchr = lchr;
      prev_rchr = rchr;
    }

    // write out the minimum current pair,
    b2g_pair_display(buffer_peek(heap_peek(pair_minheap)), out, chromosomes);
    // advance the buffer to the next sorted pair in its file,
    b2g_pair_clear(buffer_peek(heap_peek(pair_minheap)));
    buffer_consume(heap_peek(pair_minheap));
    // and refresh the heap to find the new min.
    heap_reheap_down(pair_minheap, gasv_minheap_cmp);
  }
  if (out) fclose(out);
  heap_free(pair_minheap, NULL);
}

void pair_file_buffers2gasv_input(buffer_t **fid2sorted_pairs, char *OUTPUT_PREFIX, int num_files, b2g_chromosomes_t *chromosomes, b2g_libraries_t *libraries, int WRITE_CONCORDANT, int GASV_PRO, int SPLIT_BY_CHROMOSOME) {
  int i, j;
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      if (!WRITE_CONCORDANT && CONC == j) continue; // Skip printing the buffer if we are not writing concordants.
      if (i && CONC == j && GASV_PRO) continue; // Skip printing the buffer if we are merging concordants but this is one of the libraries being merged into "all".
           
      _merge_files(fid2sorted_pairs + (i * NUM_VTYPES * num_files) + (j * num_files), num_files, chromosomes, j, OUTPUT_PREFIX, libraries, i, GASV_PRO, SPLIT_BY_CHROMOSOME);
    }
  }  
}

void *pair_file_buffers2gasv_input_start(void *args) {
  buffer_t **fid2sorted_pairs;
  char *OUTPUT_PREFIX;
  int *num_files;
  b2g_chromosomes_t *chromosomes;
  b2g_libraries_t *libraries;
  int *WRITE_CONCORDANT, *GASV_PRO, *SPLIT_BY_CHROMOSOME;

  list_values(args, &fid2sorted_pairs, &OUTPUT_PREFIX, &num_files, &chromosomes, &libraries, &WRITE_CONCORDANT, &GASV_PRO, &SPLIT_BY_CHROMOSOME);

  pair_file_buffers2gasv_input(fid2sorted_pairs, OUTPUT_PREFIX, *num_files, chromosomes, libraries, *WRITE_CONCORDANT, *GASV_PRO, *SPLIT_BY_CHROMOSOME);

  list_free(args, NULL);
  return NULL;
}

void pair_file_buffers2gasv_input_init(pthread_t *pair_file_buffers2gasv_input_thread, buffer_t **fid2sorted_pairs, char *OUTPUT_PREFIX, int *num_files, b2g_chromosomes_t *chromosomes, b2g_libraries_t *libraries, int *WRITE_CONCORDANT, int *GASV_PRO, int *SPLIT_BY_CHROMOSOME) {
  list_t *args = list(8, fid2sorted_pairs, OUTPUT_PREFIX, num_files, chromosomes, libraries, WRITE_CONCORDANT, GASV_PRO, SPLIT_BY_CHROMOSOME);
  
  pthread_create(pair_file_buffers2gasv_input_thread, NULL, pair_file_buffers2gasv_input_start, args);
}
