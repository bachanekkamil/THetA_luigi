#include <assert.h>
#include "bams2gasv_pairs.h"
#include "bam.h"
#include "sam.h"
#include "constants.h"
#include "gasv_signal.h"
#include "bam_utils.h"
#include "constants.h"
#include "gasv_pair.h"
#include "gasv_library_header.h"

void bams2gasv_pairs_test(unsigned short WRITE_LOWQ, unsigned short LIB_SEP, cutoff_queue_t cutoff_queues[]) {
  int i, j;
  char *BAM_PATH = "test/NA18507_5.bam";
  gasv_library_header_t *glib = bam_path2gasv_library_wo_cutoffs(BAM_PATH, LIB_SEP);

  int NUM_LIBS = list_length(glib->libs);
  buffer_t *lowq_pairs = buffer(10, sizeof(bam1_t));

  samfile_t *in = samopen("test/NA18507_5.bam", "rb", 0);
  LIB_SEP = 1;
  
    
  buffer_t *bams = buffer(200000, sizeof(bam1_t));
  buffer_t *lib2vtype2pairs[NUM_LIBS][NUM_VTYPES];
  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      lib2vtype2pairs[i][j] = buffer(PAIR_BUFFER_SIZE, sizeof(list_t));
    }
  }

  


  while (-1 < samread(in, buffer_buffer(bams))) {
    
    buffer_flush(bams);
  }
  gasv_signal_eof_write(buffer_buffer(bams));
  buffer_flush(bams);
  gasv_signal_eoaf_write(buffer_buffer(bams));
  buffer_flush(bams);

  

  bams2gasv_pairs(bams, lowq_pairs, &lib2vtype2pairs, glib, 1, 0, cutoff_queues);


  // Test that the returned pairs are what we expect.



  list_t *div_list = list_buffer_read((list_t *)buffer_peek(lib2vtype2pairs[0][DIV]));
  assert(gasv_signal_eof(list_ref(div_list, 0)));
    
  assert(2 == list_length(div_list));
  assert(898917 == disc_pair_lstart((disc_pair_t *)list_ref(div_list, 1)));
  list_free(div_list, gasv_pair_free);
  
  list_t *conc_list = list_buffer_read((list_t *)buffer_peek(lib2vtype2pairs[0][CONC]));
  assert(gasv_signal_eof(list_ref(conc_list, 0)));
  assert(2 == list_length(conc_list));
  assert(1201645 == conc_pair_lstart((conc_pair_t *)list_ref(conc_list, 1)));
  list_free(conc_list, gasv_pair_free);

	 
  for (i=0; i<NUM_LIBS; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      list_clear(buffer_peek(lib2vtype2pairs[i][j]), NULL);
      buffer_consume(lib2vtype2pairs[i][j]);
      assert(gasv_signal_eoaf(buffer_peek(lib2vtype2pairs[i][j])));
      buffer_free(lib2vtype2pairs[i][j], NULL);
    }
  }

  gasv_library_header_free(glib);
  buffer_free(bams, bam_clear);
  buffer_free(lowq_pairs, bam_clear);
  samclose(in);
}

void test_gasv_pair2cutoff_queue() {
  //gasv_pair2cutoff_queue(NULL, NULL, NULL, 0);
}

int main() {


  cutoff_queue_t exact_q[] = {{{EXACT, 53, 167, NULL}, 2, -1, -1, NULL}};
  bams2gasv_pairs_test(1, 1, exact_q);
  
  cutoff_queue_t sd_q[] = {{{SD, 1, -1, NULL}, 2, -1, -1, NULL}};
  bams2gasv_pairs_test(1, 1, sd_q);

  cutoff_queue_t sd2_q[] = {{{SD, 1, -1, NULL}, 10, -1, -1, NULL}};
  bams2gasv_pairs_test(1, 1, sd2_q);



  return 0;
}
