#include <stdlib.h>
#include <pthread.h>
#include "unsorted_pairs2lowq_file.h"
#include "b2g_bam.h"
#include "b2g_bam.h"
#include "b2g_constants.h"
#include "bam.h"
#include "b2g_signal.h"


void unsorted_pairs2lowq_file(buffer_t * lowq_pairs, samfile_t *lowq_file, samfile_t *split_file, int WRITE_LOWQ, int WRITE_SPLITREAD, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  assert(lowq_pairs);
  assert(lowq_file || !WRITE_LOWQ);
  assert(split_file || !WRITE_SPLITREAD);

  bam1_t *mate = NULL, *read = NULL;

  // Read a pair and
  while (!b2g_signal_eof((read = (bam1_t *)buffer_peek(lowq_pairs)))
	 && (mate = (bam1_t *)buffer_peekn(lowq_pairs, 1))) {
    assert(!b2g_signal_eof(mate)); // There must be an even number of reads.

    // either write it to splitread or
    if (WRITE_SPLITREAD && b2g_bam_pair_split((b2g_bam_t *)read, (b2g_bam_t *)mate, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) {
      assert(split_file);
      samwrite(split_file, mate);
      samwrite(split_file, read);
    }

    // to lowq, 
    else if (WRITE_LOWQ) {
      assert(lowq_file);
      samwrite(lowq_file, mate);
      samwrite(lowq_file, read);
    }      

    // then advance the queue.
    b2g_bam_clear((bam1_t *)buffer_peek(lowq_pairs));
    buffer_consume(lowq_pairs);
    b2g_bam_clear((bam1_t *)buffer_peek(lowq_pairs));
    buffer_consume(lowq_pairs);
  }
}

void *unsorted_pairs2lowq_file_start(void *args) {
  buffer_t *lowq_pairs;
  samfile_t **lowq_file, **split_file;
  int *WRITE_LOWQ, *WRITE_SPLITREAD, *MAPPING_QUALITY, *MIN_ALIGNED_PCT, *IGNORE_DUPLICATES;
  list_values(args, &lowq_pairs, &lowq_file, &split_file, &WRITE_LOWQ, &WRITE_SPLITREAD, &MAPPING_QUALITY, &MIN_ALIGNED_PCT, &IGNORE_DUPLICATES);  

  unsorted_pairs2lowq_file(lowq_pairs, *lowq_file, *split_file, *WRITE_LOWQ, *WRITE_SPLITREAD, *MAPPING_QUALITY, *MIN_ALIGNED_PCT, *IGNORE_DUPLICATES);
  list_free(args, NULL);  
  return NULL;
}

void unsorted_pairs2lowq_file_init(pthread_t *unsorted_pairs2lowq_file_thread, buffer_t * lowq_pairs, samfile_t **lowq_file, samfile_t **split_file, int *WRITE_LOWQ, int *WRITE_SPLITREAD, int *MAPPING_QUALITY, int *MIN_ALIGNED_PCT, int *IGNORE_DUPLICATES) {
  assert(unsorted_pairs2lowq_file_thread && lowq_pairs && WRITE_LOWQ && is_bool(*WRITE_LOWQ) && WRITE_SPLITREAD && is_bool(*WRITE_SPLITREAD) && MAPPING_QUALITY && in_range(*MAPPING_QUALITY, 0, 100) && MIN_ALIGNED_PCT && in_range(*MIN_ALIGNED_PCT, 50, 100) && IGNORE_DUPLICATES && is_bool(*IGNORE_DUPLICATES));

  // These are double pointers because list_t cannot handle NULL pointers as values.
  assert(*lowq_file || !*WRITE_LOWQ);
  assert(*split_file || !*WRITE_SPLITREAD);

  pthread_create(unsorted_pairs2lowq_file_thread, NULL, unsorted_pairs2lowq_file_start, list(8, lowq_pairs, lowq_file, split_file, WRITE_LOWQ, WRITE_SPLITREAD, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
}
