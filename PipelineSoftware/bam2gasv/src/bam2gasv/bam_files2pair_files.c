#include <assert.h>
#include <signal.h>
#include "bam.h"
#include "bam_files2pair_files.h"
#include "buffer.h"
#include "bam_files2bams.h"
#include "b2g_constants.h"
#include "bams2unsorted_pairs.h"
#include "b2g_bam.h"
#include "b2g_signal.h"
#include "b2g_pair.h"
#include "unsorted_pairs2pairs.h"
#include "pairs2pair_files.h"
#include "unsorted_pairs2lowq_file.h"
#include "b2g_libraries.h"
#include "b2g_chromosome.h"


static void _alias_unsorted_array(list_t *file2lib2vtype2unsorted_pairs, list_t **vtype2file2lib2unsorted_pairs, int num_files, int NUM_LIBS) {
  int i, j, k;  
  bzero(file2lib2vtype2unsorted_pairs, sizeof(list_t) * num_files * NUM_LIBS * NUM_VTYPES);
  for (i=0; i<num_files; i++) {
    for (j=0; j<NUM_LIBS; j++) {
      for (k=0; k<NUM_VTYPES; k++) {
	*(vtype2file2lib2unsorted_pairs + (k * num_files * NUM_LIBS) + (i * NUM_LIBS) + j) = file2lib2vtype2unsorted_pairs + (i * NUM_LIBS * NUM_VTYPES) + (j * NUM_VTYPES) + k;
      }
    }
  }
}

b2g_error_t bam_files2pair_files(int num_files, char *BAM_PATH, char *OUTPUT_PREFIX, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes,   hash_t *unpaired, samfile_t *lowq_file, samfile_t *split_file, int WRITE_LOWQ, int MAPPING_QUALITY, int LOW_MEMORY, int PROPER_LENGTH, int AMBIGUOUS, int VERBOSE, int WRITE_CONCORDANT, int DEBUG_LEVEL, int WRITE_SPLITREAD, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int USE_NUMBER_READS, int GASV_PRO, int QNAME_SORTED, int SPLIT_BY_CHROMOSOME, unsigned int *num_skipped) {
  assert(is_positive(num_files) && BAM_PATH && OUTPUT_PREFIX && libraries && is_bool(WRITE_LOWQ) && in_range(MAPPING_QUALITY, 0, 255) && is_bool(LOW_MEMORY) && is_nonnegative(PROPER_LENGTH) && is_bool(AMBIGUOUS) && is_bool(VERBOSE) && is_bool(WRITE_CONCORDANT) && is_bool(WRITE_SPLITREAD) && in_range(MIN_ALIGNED_PCT, 50, 100) && is_bool(IGNORE_DUPLICATES) && is_positive(USE_NUMBER_READS) && is_bool(QNAME_SORTED));  

  int i;
  int NUM_LIBS = b2g_libraries_length(libraries);

  buffer_t *bams = buffer(BAM_FILES2BAMS_BUFFER, sizeof(bam1_t)); // bam1_t's from BAM file, B2G_SIGNAL_EOF between files.
  buffer_t *lowq_pairs = buffer(LOWQ_BUFFER_SIZE, sizeof(bam1_t)); // lowq bam1_t's, B2G_SIGNAL_EOF when complete.

  // Each holds one list_t per BAM file, the tail of which contains the pairs from the bam file (head is reserved for EOF). The list_t begins with B2G_SIGNAL_EOF when that file is done.
  list_t file2lib2vtype2unsorted_pairs[num_files][NUM_LIBS][NUM_VTYPES];  
  list_t *vtype2file2lib2unsorted_pairs[NUM_VTYPES][num_files][NUM_LIBS];  

  _alias_unsorted_array(&file2lib2vtype2unsorted_pairs[0][0][0], &vtype2file2lib2unsorted_pairs[0][0][0], num_files, NUM_LIBS);

  buffer_t *vtype2pairs[NUM_VTYPES];
  for (i=0; i<NUM_VTYPES; i++) vtype2pairs[i] = buffer(num_files * NUM_LIBS, sizeof(list_t));
  
  pthread_t bams2unsorted_pairs_thread, 
unsorted_pairs2lowq_file_thread;
  pthread_t unsorted_pairs2pairs_threads[NUM_VTYPES];
  pthread_t pairs2pair_files_threads[NUM_VTYPES];
  
  // Start the various worker threads,

  // Consumes BAM reads, produces pairs.
  bams2unsorted_pairs_init(&bams2unsorted_pairs_thread, bams, lowq_pairs, &file2lib2vtype2unsorted_pairs[0][0][0], libraries, chromosomes, unpaired, &WRITE_LOWQ, &MAPPING_QUALITY, &PROPER_LENGTH, &AMBIGUOUS, &WRITE_CONCORDANT, &DEBUG_LEVEL, &MIN_ALIGNED_PCT, &IGNORE_DUPLICATES, &PLATFORM, &VERBOSE, &USE_NUMBER_READS, &GASV_PRO, &num_files, &QNAME_SORTED, &WRITE_SPLITREAD, &LOW_MEMORY, num_skipped);  
  
  // Consumes pairs, produces the lowq and splitread files.
  if (WRITE_LOWQ || WRITE_SPLITREAD) unsorted_pairs2lowq_file_init(&unsorted_pairs2lowq_file_thread, lowq_pairs, &lowq_file, &split_file, &WRITE_LOWQ, &WRITE_SPLITREAD, &MAPPING_QUALITY, &MIN_ALIGNED_PCT, &IGNORE_DUPLICATES);
  
  // Consumes pairs, produces sorted lists of pairs.
  unsorted_pairs2pairs_init(unsorted_pairs2pairs_threads, &NUM_LIBS, &vtype2file2lib2unsorted_pairs[0][0][0], vtype2pairs, &DEBUG_LEVEL, WRITE_CONCORDANT, &GASV_PRO, &num_files);

  // Consumes sorted lists of pairs, produces gasv output files.
  pairs2pair_files_init(pairs2pair_files_threads, vtype2pairs, OUTPUT_PREFIX, NUM_LIBS, WRITE_CONCORDANT, &LOW_MEMORY, libraries, chromosomes, &num_files, &GASV_PRO, &VERBOSE, &SPLIT_BY_CHROMOSOME);

  // and become the bam_files2bams input reader thread.
  bam_files2bams(BAM_PATH, OUTPUT_PREFIX, num_files, LOW_MEMORY, bams, chromosomes, VERBOSE, num_skipped);

  // Join the other threads when all input has been read.
  pthread_join(bams2unsorted_pairs_thread, NULL);			      

  for (i=0; i<NUM_VTYPES; i++) { 
    if (!WRITE_CONCORDANT && CONC == i) continue;
    pthread_join(unsorted_pairs2pairs_threads[i], NULL);
  }
  for (i=0; i<NUM_VTYPES; i++) {
    if (!WRITE_CONCORDANT && CONC == i) continue;
    pthread_join(pairs2pair_files_threads[i], NULL);
  }
  if (WRITE_LOWQ || WRITE_SPLITREAD) pthread_join(unsorted_pairs2lowq_file_thread, NULL);

  buffer_free(bams, b2g_bam_clear);  
  buffer_free(lowq_pairs, NULL);
  for (i=0; i<NUM_VTYPES; i++) buffer_free(vtype2pairs[i], NULL);
  return B2GERR_NO_ERROR;
}
