#include <pthread.h>
#include "pair_files2gasv_input.h"
#include "b2g_constants.h"
#include "b2g_pair.h"
#include "buffer.h"
#include "list.h"
#include "pair_files2pair_file_buffers.h"
#include "pair_file_buffers2gasv_input.h"
#include "b2g_variant_type.h"
#include "b2g_chromosome.h"
#include "b2g_libraries.h"

void pair_files2gasv_input(char *OUTPUT_PREFIX, int num_files, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, int WRITE_CONCORDANT, int GASV_PRO, int SPLIT_BY_CHROMOSOME) {
  
  int i, j, k;
  buffer_t *fid2sorted_pairs[b2g_libraries_length(libraries)][NUM_VTYPES][num_files];
  pthread_t pair_files2pair_file_buffers_thread;
  pthread_t pair_file_buffers2gasv_input_thread;

  // Create a buffer for every (library x variant_type x split_file).
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      if (!WRITE_CONCORDANT && CONC == j) continue;
      for (k=0; k<num_files; k++) {
	fid2sorted_pairs[i][j][k] = buffer(FILES2OUTPUT_BUFFER_SIZE, gasv_sizeof(j));
      }
    }
  }

  // Create threads to read files into those buffers and write them back out.
  pair_files2pair_file_buffers_init(&pair_files2pair_file_buffers_thread, &fid2sorted_pairs[0][0][0], OUTPUT_PREFIX, &num_files, libraries, &WRITE_CONCORDANT, &GASV_PRO);
  pair_file_buffers2gasv_input_init(&pair_file_buffers2gasv_input_thread, &fid2sorted_pairs[0][0][0], OUTPUT_PREFIX, &num_files, chromosomes, libraries, &WRITE_CONCORDANT, &GASV_PRO, &SPLIT_BY_CHROMOSOME);
  
  // Cleanup.
  pthread_join(pair_files2pair_file_buffers_thread, NULL);
  pthread_join(pair_file_buffers2gasv_input_thread, NULL);
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      if (!WRITE_CONCORDANT && CONC == j) continue;   
      for (k=0; k<num_files; k++) {	
	buffer_free(fid2sorted_pairs[i][j][k], NULL); 
      }
    }
  }
}



