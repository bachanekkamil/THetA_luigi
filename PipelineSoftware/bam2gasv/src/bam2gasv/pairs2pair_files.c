#include <stdio.h>
#include <assert.h>
#include <pthread.h>
#include "pairs2pair_files.h"
#include "list.h"
#include "buffer.h"
#include "b2g_pair.h"
#include "io.h"
#include "b2g_variant_type.h"
#include "integer.h"
#include "b2g_libraries.h"
#include "b2g_chromosome.h"
#include "b2g_pair_file.h"

static void _pairs_write_low_memory(list_t *sorted_pairs, char *OUTPUT_PREFIX, b2g_variant_t vtype, int lib, int file_num) {
  assert(sorted_pairs && OUTPUT_PREFIX);

  FILE *out = file_open("%s_%d_%d.%s", "w", OUTPUT_PREFIX, lib, file_num, VARIANT_EXTENSIONS[vtype]);

  while (sorted_pairs && sorted_pairs->car) {
    b2g_pair_write(sorted_pairs->car, out);
    sorted_pairs = sorted_pairs->cdr;
  }
  fclose(out);
}

static void _pairs_write(list_t *sorted_pairs, char *OUTPUT_PREFIX, b2g_chromosomes_t *chromosomes, int lib, b2g_variant_t vtype, b2g_libraries_t *libraries, int GASV_PRO, int VERBOSE, int SPLIT_BY_CHROMOSOME) {
  assert(sorted_pairs);
  FILE *out = NULL;
  int lchr = -1, rchr = -1, prev_lchr = -1, prev_rchr = -1;

  while (sorted_pairs && sorted_pairs->car) {
    lchr = b2g_pair_lchr((b2g_pair_t *)sorted_pairs->car);
    rchr = b2g_pair_rchr((b2g_pair_t *)sorted_pairs->car);
    if (lchr != prev_lchr || rchr != prev_rchr) {
      if (out) fclose(out);
      out = _open_gasv_input_file(OUTPUT_PREFIX, libraries, lib, vtype, GASV_PRO, SPLIT_BY_CHROMOSOME, lchr, rchr);
      prev_lchr = lchr;
      prev_rchr = rchr;
    }
    b2g_pair_display(sorted_pairs->car, out, chromosomes);
    sorted_pairs = sorted_pairs->cdr;
  }
  if (out) fclose(out);
  else if (VERBOSE) printf("No %s pairs in BAM file: omitting from output\n", VARIANT_EXTENSIONS[vtype]);
}

void pairs2pair_files(buffer_t *pairs, char *OUTPUT_PREFIX, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, int vtype, int num_files, int LOW_MEMORY, int GASV_PRO, int VERBOSE, int SPLIT_BY_CHROMOSOME) {
  int i, j;
  list_t *sorted_pairs;

  for (i=0; i<num_files; i++) {
    for (j=0; j<b2g_libraries_length(libraries); j++) {
      if (j && CONC == vtype && GASV_PRO) continue;

      sorted_pairs = buffer_peek_lock(pairs);

      // Unless we are on LOW_MEMORY mode, this IS the final output.
      if (LOW_MEMORY) _pairs_write_low_memory(sorted_pairs, OUTPUT_PREFIX, vtype, j, i);
      else _pairs_write(sorted_pairs, OUTPUT_PREFIX, chromosomes, j, vtype, libraries, GASV_PRO, VERBOSE, SPLIT_BY_CHROMOSOME);
      list_clear(sorted_pairs, b2g_pair_free);

      buffer_consume(pairs);
      
    }
  }
}

void *pairs2pair_files_start(void *args) {

  buffer_t *pairs;
  char *OUTPUT_PREFIX;
  int *vtype, *LOW_MEMORY, *num_files, *GASV_PRO, *VERBOSE, *SPLIT_BY_CHROMOSOME;
  b2g_libraries_t *libraries;
  b2g_chromosomes_t *chromosomes;

  list_values(args, &pairs, &OUTPUT_PREFIX, &vtype, &LOW_MEMORY, &libraries, &chromosomes, &num_files, &GASV_PRO, &VERBOSE, &SPLIT_BY_CHROMOSOME);

  pairs2pair_files(pairs, OUTPUT_PREFIX, libraries, chromosomes, *vtype, *num_files, *LOW_MEMORY, *GASV_PRO, *VERBOSE, *SPLIT_BY_CHROMOSOME);

  list_free(args, NULL);
  return NULL;
}

void pairs2pair_files_init(pthread_t pairs2pair_files_threads[NUM_VTYPES], buffer_t *vtype2pairs[NUM_VTYPES], char *OUTPUT_PREFIX, int NUM_LIBS, int WRITE_CONCORDANT, int *LOW_MEMORY, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, int *num_files, int *GASV_PRO, int *VERBOSE, int *SPLIT_BY_CHROMOSOME) {
  assert(OUTPUT_PREFIX && is_positive(NUM_LIBS) && is_bool(WRITE_CONCORDANT) && LOW_MEMORY && is_bool(*LOW_MEMORY) && libraries && chromosomes && num_files && in_range(*num_files, 1, MAX_BAM_FILES) && GASV_PRO && is_bool(*GASV_PRO) && VERBOSE && is_bool(*VERBOSE) && SPLIT_BY_CHROMOSOME && is_bool(*SPLIT_BY_CHROMOSOME));

  int i;
  for (i=0; i<NUM_VTYPES; i++) {
    if (!WRITE_CONCORDANT && CONC == i) continue;
    pthread_create(&pairs2pair_files_threads[i], NULL, pairs2pair_files_start, list(10, vtype2pairs[i], OUTPUT_PREFIX, &VARIANT_TYPES[i], LOW_MEMORY, libraries, chromosomes, num_files, GASV_PRO, VERBOSE, SPLIT_BY_CHROMOSOME));

  }
}
