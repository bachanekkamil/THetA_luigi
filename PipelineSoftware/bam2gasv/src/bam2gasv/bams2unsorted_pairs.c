#include <pthread.h>
#include <assert.h>
#include "bams2unsorted_pairs.h"
#include "buffer.h"
#include "b2g_constants.h"
#include "b2g_variant_type.h"
#include "bam.h"
#include "hash.h"
#include "b2g_signal.h"
#include "b2g_bam.h"
#include "b2g_pair.h"
#include "integer.h"
#include "b2g_libraries.h"
#include "b2g_compressed_bam.h"



// Collects the previous bam or bams for use in creating pairs in QNAME_SORTED mode.
union bam_or_bams {
  b2g_bam_t *bam;
  list_t *bams;
};

// After a complete file has been read, we know that all possible pairs in that file have been paired. We can therefore flush the pairs down the pipe and clear our memory for a new file.
static void _write_eofs(list_t *lib2vtype2unsorted_pairs, unsigned int NUM_LIBS, hash_t *unpaired) {
  int i, j;
  list_t *unsorted_pairs;
  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      unsorted_pairs = lib2vtype2unsorted_pairs + (i * NUM_VTYPES) + j;
      b2g_signal_eof_write(&unsorted_pairs->car);
    }
  }  
}

// Look up all bams with matching qname in the hash table.
/*TODO
union bam_or_bams hash2bams(hash_t *unpaired, b2g_bam_t *bam1, int WRITE_LOWQ, int AMBIGUOUS) {
  union bam_or_bams matches;
  if (!AMBIGUOUS) {
    matches.bam = hash_remsert(unpaired, bam1, djb2_hash(b2g_bam_qname(bam1)), b2g_bam_qname_cmp, (void*)b2g_bam_clone);
  }
  else { // if AMBIGUOUS
    list_t *bam2s = NULL, *bucket  = hash_bucket(unpaired, djb2_hash(b2g_bam_qname(bam1)));

    bam2s = list_findall(bucket, bam1, b2g_bam_qname_cmp);
    list_merge_into(bucket, list(1, b2g_bam_clone((bam1_t *)bam1)), b2g_bam_qname_cmp);
    bam2s ? bam2s : list(0);
    matches.bams = bam2s;
  }
  return matches;
}
*/

// Sets bam2s to the previous bams if they match the qname of bam1, otherwise stores bam1 for comparison to the bams after it and sets bam2s to NULL. Assumes the bams will arrive in qname-sorted order and returns B2GERR_UNSORTED if this is violated. 
list_t *_matching_previous_bams(b2g_bam_t *bam1, list_t **previous_bams, unsigned int *num_skipped, int LOW_MEMORY, b2g_error_t *ERR) {
  int cmp;
  list_t *bam2s = NULL;

  if (!*previous_bams) *previous_bams = cons(b2g_bam_clone(bam1), NULL);
  else if (!(cmp = b2g_bam_qname_cmp(bam1, (*previous_bams)->car))) {
    bam2s = *previous_bams;
    *previous_bams = cons(b2g_bam_clone(bam1), *previous_bams);
  }
  else if (0 < cmp) {
    (*num_skipped) += list_free(*previous_bams, b2g_bam_free);
    *previous_bams = cons(b2g_bam_clone(bam1), NULL);
  }
  else *ERR = B2GERR_UNSORTED;
  return bam2s;
}

// Sets bam2 to the previous bam if it matches the qname of bam1, otherwise stores bam1 for comparison to the bam after it and sets bam2 to NULL. Assumes the bams will arrive in qname-sorted order and returns B2GERR_UNSORTED if this is violated. 
b2g_bam_t *_matching_previous_bam(b2g_bam_t *bam1, b2g_bam_t **previous_bam, unsigned int *num_skipped, int LOW_MEMORY, b2g_error_t *ERR) {
  int cmp;
  b2g_bam_t *bam2 = NULL;

  // If there is no previous bam, store this one.
  if (!*previous_bam) *previous_bam = b2g_bam_clone(bam1);
  // Otherwise, if we have a match,
  else if (!(cmp = b2g_bam_qname_cmp(bam1, *previous_bam))) {
    // return it,
    bam2 = *previous_bam;
    *previous_bam = NULL;
  }
  // and if we dont, store the new previous.
  else if (0 < cmp) {
    (*num_skipped) += !!*previous_bam;
    b2g_bam_free(*previous_bam);
    *previous_bam = b2g_bam_clone(bam1);
  }
  else *ERR = B2GERR_UNSORTED;
  return bam2;
}


// Consume a bam and attempt to pair it and write it to the appropriate library buffer.
static b2g_error_t _consume_bam(b2g_bam_t *bam1, hash_t *unpaired, union bam_or_bams *previous_bams, int WRITE_LOWQ, int AMBIGUOUS, int DEBUG_LEVEL, b2g_libraries_t *libraries, list_t *lib2vtype2unsorted_pairs, buffer_t *lowq_pairs, b2g_chromosomes_t *chromosomes, int MAPPING_QUALITY, int PROPER_LENGTH, int WRITE_CONCORDANT, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int VERBOSE, int USE_NUMBER_READS, int GASV_PRO, int QNAME_SORTED, int WRITE_SPLITREAD, int LOW_MEMORY, unsigned int *num_skipped) {
  assert(b2g_bam_qname(bam1));
  b2g_error_t ERR = B2GERR_NO_ERROR;  
  b2g_extended_bam_t extended_bam = {MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, bam1};

  if (AMBIGUOUS) b2g_libraries_write_ambiguous(libraries, bam1, _matching_previous_bams(bam1, &previous_bams->bams, num_skipped, LOW_MEMORY, &ERR), lib2vtype2unsorted_pairs, lowq_pairs, chromosomes, MAPPING_QUALITY, PROPER_LENGTH, WRITE_CONCORDANT, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO);
    
  else if (!AMBIGUOUS) { // If we are in unambiguous mode, get the matching bam2 either

    if (QNAME_SORTED) b2g_libraries_write(libraries, b2g_bam_qname(bam1), bam1, _matching_previous_bam(bam1, &previous_bams->bam, num_skipped, LOW_MEMORY, &ERR), lib2vtype2unsorted_pairs, lowq_pairs, chromosomes, WRITE_LOWQ, MAPPING_QUALITY, PROPER_LENGTH, WRITE_CONCORDANT, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO, WRITE_SPLITREAD);

    else if (WRITE_LOWQ || WRITE_SPLITREAD) b2g_libraries_write(libraries, b2g_bam_qname(bam1), bam1, hash_remsert(unpaired, bam1, djb2_hash(b2g_bam_qname(bam1)), b2g_bam_qname_cmp, (void*)b2g_bam_clone), lib2vtype2unsorted_pairs, lowq_pairs, chromosomes, WRITE_LOWQ, MAPPING_QUALITY, PROPER_LENGTH, WRITE_CONCORDANT, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO, WRITE_SPLITREAD);

    // QNAME_SORTED doesn't bother with compression at the moment and WRITE_LOWQ requires us to have the whole read on hand in case it is a lowq pair. Hence, noly if neither of these is true do we use compression.
    else if (!QNAME_SORTED && !WRITE_LOWQ && !WRITE_SPLITREAD) b2g_libraries_write_compressed(libraries, b2g_bam_qname(bam1), bam1, hash_remsert(unpaired, &extended_bam, djb2_hash(b2g_bam_qname(bam1)), b2g_compressed_bam_qname_cmp, (void*)b2g_extended_bam2compressed_bam), lib2vtype2unsorted_pairs, chromosomes, MAPPING_QUALITY, PROPER_LENGTH, WRITE_CONCORDANT, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO);
  }
  return ERR;
}

b2g_error_t bams2unsorted_pairs(buffer_t *bams, buffer_t *lowq_pairs, list_t *file2lib2vtype2unsorted_pairs, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, hash_t *unpaired, unsigned short WRITE_LOWQ, int MAPPING_QUALITY, int PROPER_LENGTH, int AMBIGUOUS, int WRITE_CONCORDANT, int DEBUG_LEVEL, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int VERBOSE, int USE_NUMBER_READS, int GASV_PRO, int num_files, int QNAME_SORTED, int WRITE_SPLITREAD, int LOW_MEMORY, unsigned int *num_skipped) {

  int i, num_cleared;
  b2g_bam_t *bam1;
  b2g_error_t ERR = B2GERR_NO_ERROR;  
  union bam_or_bams previous_bams ={NULL};
  // For each BAM file,
  for (i=0; i<num_files; i++) {
    // consume all bams up to an EOF signal, and then
    while (!b2g_signal_eof(bam1 = (b2g_bam_t*)buffer_peek(bams))) {
      if ((ERR = _consume_bam(bam1, unpaired, &previous_bams, WRITE_LOWQ, AMBIGUOUS, DEBUG_LEVEL, libraries, file2lib2vtype2unsorted_pairs + (i * NUM_VTYPES * b2g_libraries_length(libraries)), lowq_pairs, chromosomes, MAPPING_QUALITY, PROPER_LENGTH, WRITE_CONCORDANT, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO, QNAME_SORTED, WRITE_SPLITREAD, LOW_MEMORY, num_skipped))) return ERR;
      buffer_consume(bams);
    }

    // free any previous bams and,
    if (AMBIGUOUS) {
      (*num_skipped) += list_free(previous_bams.bams, NULL);
      previous_bams.bams = NULL;
    }
    else {
      (*num_skipped) += !!previous_bams.bam;
      b2g_bam_free(previous_bams.bam);
      previous_bams.bam = NULL;
    }
      

    // while guaranteeing that libraries are flushed, even if we have too few bams to meet USE_NUMBER_READS,
    if (i + 1 == num_files) b2g_libraries_flush(libraries, file2lib2vtype2unsorted_pairs + (i * NUM_VTYPES * b2g_libraries_length(libraries)), PROPER_LENGTH, WRITE_CONCORDANT, PLATFORM, VERBOSE);
    // pass the EOF signals along.
    _write_eofs(file2lib2vtype2unsorted_pairs + (i * NUM_VTYPES * b2g_libraries_length(libraries)), b2g_libraries_length(libraries), unpaired);
    num_cleared = hash_clear(unpaired, b2g_compression_on(QNAME_SORTED, WRITE_LOWQ) ? b2g_compressed_bam_free : (void (*)(void *))b2g_bam_free);
    (*num_skipped) += num_cleared;
    buffer_consume(bams);
  }

  assert(lowq_pairs);
  b2g_signal_eof_write(buffer_buffer(lowq_pairs));
  buffer_flush(lowq_pairs);
  if (AMBIGUOUS) list_free(previous_bams.bams, b2g_bam_free);
  else b2g_bam_free(previous_bams.bam);
  return ERR;
}


void *bams2unsorted_pairs_start(void *args) {
  list_t *arg_list = (list_t *)args;
  buffer_t* bams;
  buffer_t *lowq_pairs;
  list_t *file2lib2vtype2unsorted_pairs;
  b2g_libraries_t *libraries;
  int *WRITE_LOWQ, *MAPPING_QUALITY, *PROPER_LENGTH, *AMBIGUOUS, *WRITE_CONCORDANT, *DEBUG_LEVEL, *MIN_ALIGNED_PCT, *IGNORE_DUPLICATES, *VERBOSE, *USE_NUMBER_READS, *GASV_PRO, *num_files, *QNAME_SORTED, *WRITE_SPLITREAD, *LOW_MEMORY;
  b2g_platform_t *PLATFORM;
  b2g_chromosomes_t *chromosomes;
  b2g_error_t ERR = B2GERR_NO_ERROR;
  hash_t *unpaired;
  unsigned int *num_skipped;

  list_values(arg_list, &bams, &lowq_pairs, &file2lib2vtype2unsorted_pairs, &libraries, &chromosomes, &unpaired, &WRITE_LOWQ, &MAPPING_QUALITY, &PROPER_LENGTH, &AMBIGUOUS, &WRITE_CONCORDANT, &DEBUG_LEVEL, &MIN_ALIGNED_PCT, &IGNORE_DUPLICATES, &PLATFORM, &VERBOSE, &USE_NUMBER_READS, &GASV_PRO, &num_files, &QNAME_SORTED, &WRITE_SPLITREAD, &LOW_MEMORY, &num_skipped);

  ERR = bams2unsorted_pairs(bams, lowq_pairs, file2lib2vtype2unsorted_pairs, libraries, chromosomes, unpaired, *WRITE_LOWQ, *MAPPING_QUALITY, *PROPER_LENGTH, *AMBIGUOUS, *WRITE_CONCORDANT, *DEBUG_LEVEL, *MIN_ALIGNED_PCT, *IGNORE_DUPLICATES, *PLATFORM, *VERBOSE, *USE_NUMBER_READS, *GASV_PRO, *num_files, *QNAME_SORTED, *WRITE_SPLITREAD, *LOW_MEMORY, num_skipped);
  list_free(arg_list, NULL);

  if (ERR) b2g_exit(ERR);
  return NULL;
}


void bams2unsorted_pairs_init(pthread_t *bams2unsorted_pairs_thread, buffer_t *bams, buffer_t *lowq_pairs, list_t *file2lib2vtype2unsorted_pairs, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, hash_t *unpaired, int *WRITE_LOWQ, int *MAPPING_QUALITY, int *PROPER_LENGTH, int *AMBIGUOUS, int *WRITE_CONCORDANT, int *DEBUG_LEVEL, int *MIN_ALIGNED_PCT, int *IGNORE_DUPLICATES, b2g_platform_t *PLATFORM, int *VERBOSE, int *USE_NUMBER_READS, int *GASV_PRO, int *num_files, int *QNAME_SORTED, int *WRITE_SPLITREAD, int *LOW_MEMORY, unsigned int *num_skipped) {
  assert(bams2unsorted_pairs_thread && bams && lowq_pairs && libraries && chromosomes && WRITE_LOWQ && is_bool(*WRITE_LOWQ) && MAPPING_QUALITY && in_range(*MAPPING_QUALITY, 0, 100) && is_nonnegative(*PROPER_LENGTH) && AMBIGUOUS && is_bool(*AMBIGUOUS) && WRITE_CONCORDANT && is_bool(*WRITE_CONCORDANT) && MIN_ALIGNED_PCT && is_nonnegative(*MIN_ALIGNED_PCT) && IGNORE_DUPLICATES && is_bool(*IGNORE_DUPLICATES) && PLATFORM && VERBOSE && is_bool(*VERBOSE) && USE_NUMBER_READS && is_positive(USE_NUMBER_READS) && num_files && is_positive(num_files) && unpaired && WRITE_SPLITREAD && is_bool(*WRITE_SPLITREAD) && LOW_MEMORY && is_bool(*LOW_MEMORY));  

  pthread_create(bams2unsorted_pairs_thread, NULL, bams2unsorted_pairs_start, list(23, bams, lowq_pairs, file2lib2vtype2unsorted_pairs, libraries, chromosomes, unpaired, WRITE_LOWQ, MAPPING_QUALITY, PROPER_LENGTH, AMBIGUOUS, WRITE_CONCORDANT, DEBUG_LEVEL, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, VERBOSE, USE_NUMBER_READS, GASV_PRO, num_files, QNAME_SORTED, WRITE_SPLITREAD, LOW_MEMORY, num_skipped));
}
