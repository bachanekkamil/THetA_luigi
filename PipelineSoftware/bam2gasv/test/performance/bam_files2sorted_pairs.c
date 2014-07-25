#include "bam_files2bams.h"
#include "buffer.h"
#include "bam.h"
#include "bam_utils.h"
#include "constants.h"
#include "cutoff_lminlmax.h"
#include "gasv_library.h"
#include "bams2gasv_pairs.h"
#include "gasv_pairs2sorted_pairs.h"
#include <time.h>



int main(int argc, char **argv) {
  
  int i, j;

  time_t start = time(NULL);

  // parameters
  int WRITE_LOWQ = 0;
  int MAPPING_QUALITY = 10;
  int PROPER_LENGTH = 10000;
  int AMBIGUOUS = 0;
  int WRITE_CONCORDANT = 0;
  int LIB_SEP = 0;
  int USE_NUMBER_READS = 500000;
  int NUM_LIBS = 1;
  int DEBUG_LEVEL = 2;

  library_cutoff_t cutoff = {PCT, 99, -1, NULL};

  gasv_library_t *glib = bam_path2gasv_library(argv[1], LIB_SEP, &cutoff, USE_NUMBER_READS);

  // buffers
  buffer_t *bams = buffer(BAM_FILES2BAMS_BUFFER, sizeof(bam1_t));
  buffer_t *lowq_pairs = buffer(LOWQ_BUFFER_SIZE, sizeof(bam1_t));
  buffer_t *lib2vtype2gasv_pairs[NUM_LIBS][NUM_VTYPES];
  buffer_t *lib2vtype2sorted_pairs[NUM_LIBS][NUM_VTYPES];
  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      lib2vtype2gasv_pairs[i][j] = buffer(PAIR_BUFFER_SIZE, sizeof(list_t));
      lib2vtype2sorted_pairs[i][j] = buffer(PAIR_BUFFER_SIZE, sizeof(list_t));
    }
  }

  // threads
  pthread_t bams2gasv_pairs_thread;
  pthread_t gasv_pair2sorted_pairs_threads[NUM_LIBS][NUM_VTYPES];
  bams2gasv_pairs_init(&bams2gasv_pairs_thread, bams, lowq_pairs, &lib2vtype2gasv_pairs, glib, &WRITE_LOWQ, &MAPPING_QUALITY, &PROPER_LENGTH, &AMBIGUOUS, &WRITE_CONCORDANT, &DEBUG_LEVEL);  

gasv_pairs2sorted_pairs_init(&gasv_pair2sorted_pairs_threads, NUM_LIBS, &lib2vtype2gasv_pairs, &lib2vtype2sorted_pairs, &DEBUG_LEVEL);

  bam_files2bams(argv[1], NULL, 1, 0, bams, 0, 1);


  pthread_join(bams2gasv_pairs_thread, NULL);

  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) { 
      pthread_join(gasv_pair2sorted_pairs_threads[i][j], NULL);
    }
  }

  buffer_free(bams, bam_clear);
  buffer_free(lowq_pairs, NULL);

  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      //list_clear(buffer_peek(lib2vtype2gasv_pairs[i][j]), NULL);
      buffer_free(lib2vtype2gasv_pairs[i][j], NULL);
      buffer_free(lib2vtype2sorted_pairs[i][j], NULL);
    }
  }
  
  gasv_library_free(glib);


  printf("%f\n", difftime(time(NULL), start));

  return 0;
}
