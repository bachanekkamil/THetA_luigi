#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include "pair_files2pair_file_buffers.h"
#include "list.h"
#include "b2g_signal.h"
#include "b2g_variant_type.h"
#include "buffer.h"
#include "io.h"
#include "b2g_pair.h"
#include "integer.h"
#include "b2g_libraries.h"

// Fills a buffer to its limit with pairs from the given file.
// Returns B2G_SIGNAL_EOF if the file is done, or the new file offset if it is not.
int fill_pair_buffer(buffer_t *sorted_pairs, char *OUTPUT_PREFIX, int lib, b2g_variant_t vtype, int fno, int foffset) {
  if (B2G_SIGNAL_EOF == foffset) return B2G_SIGNAL_EOF;
  assert(sorted_pairs && is_nonnegative(foffset) && is_nonnegative(lib) && is_nonnegative(fno));

  int total_read = 0, amt_read;
  char filename[MAX_FILENAME_SIZE] = {};
  sprintf(filename, "%s_%d_%d.%s", OUTPUT_PREFIX, lib, fno, VARIANT_EXTENSIONS[vtype]);
  FILE *in = file_open(filename, "r");
  
  if (!in) return B2G_SIGNAL_EOF;
  //printf("reading from: %s_%d_%d.%s at %d\n", OUTPUT_PREFIX, lib, fno, VARIANT_EXTENSIONS[vtype], foffset);
  fseek(in, foffset, SEEK_SET);

  // b2g_pair_read returns 0 if the buffer is full and B2G_SIGNAL_EOF if the file is empty.
  while (0 < (amt_read = b2g_pair_read(buffer_buffer_try(sorted_pairs), in, vtype))) {    
    //assert(vtype == b2g_pair_vtype(buffer_buffer(sorted_pairs)));
    assert((CONC == vtype) == b2g_conc_pairp((b2g_pair_t *)buffer_buffer(sorted_pairs)));
    total_read += amt_read;
    buffer_flush(sorted_pairs);
  }
  fclose(in);

  if (B2G_SIGNAL_EOF == amt_read) {
    unlink(filename);
    return B2G_SIGNAL_EOF;
  }
  else return total_read + foffset;
}

void _library_vtype2buffers(buffer_t *fid2sorted_pairs[], char *OUTPUT_PREFIX, int num_files, int lib, b2g_variant_t vtype) {
  
  int fid;
  int pairs_remain = 1;
  int fid2foffset[num_files];
  bzero(fid2foffset, num_files * sizeof(int));

  // While more pairs remain in any file,
  while (pairs_remain) {
    pairs_remain = 0;
    for (fid=0; fid<num_files; fid++) {
      // find a fileid with more pairs remaining and
      if (B2G_SIGNAL_EOAF == fid2foffset[fid]) continue;

      pairs_remain = 1;

      // if, by reading that file in, we exhaust it,
      if (B2G_SIGNAL_EOF == (fid2foffset[fid] = fill_pair_buffer(fid2sorted_pairs[fid], OUTPUT_PREFIX, lib, vtype, fid, fid2foffset[fid]))) {

	// pass on the EOF and eliminate the file from our list.
	if (!buffer_full(fid2sorted_pairs[fid])) {
	  fid2foffset[fid] = B2G_SIGNAL_EOAF;
	  b2g_signal_eof_write(buffer_buffer(fid2sorted_pairs[fid]));	  
	  buffer_flush(fid2sorted_pairs[fid]);
	}
      }
    }
  }
}

void _pair_files2pair_file_buffers(buffer_t **fid2sorted_pairs, char *OUTPUT_PREFIX, int num_files, b2g_libraries_t *libraries, int WRITE_CONCORDANT, int GASV_PRO) {
  int i, j;
  for (i=0; i<b2g_libraries_length(libraries); i++) { 
    for (j=0; j<NUM_VTYPES; j++) {
      if (CONC == j && !WRITE_CONCORDANT) continue; // Skip reading the file if we are not writing concordants.
      if (i && CONC == j && GASV_PRO) continue; // Skip reading the file if we are merging concordants but this is one of the libraries being merged into "all".
      _library_vtype2buffers(fid2sorted_pairs + (i * NUM_VTYPES * num_files) + (j * num_files), OUTPUT_PREFIX, num_files, i, j);
    }    
  }  
}

void *pair_files2pair_file_buffers_start(void *args) {
  
  buffer_t **fid2sorted_pairs;
  char *OUTPUT_PREFIX;
  int *num_files;
  b2g_libraries_t *libraries;
  int *WRITE_CONCORDANT, *GASV_PRO;
  list_values(args, &fid2sorted_pairs, &OUTPUT_PREFIX, &num_files, &libraries, &WRITE_CONCORDANT, &GASV_PRO);

  _pair_files2pair_file_buffers(fid2sorted_pairs, OUTPUT_PREFIX, *num_files, libraries, *WRITE_CONCORDANT, *GASV_PRO);

  list_free(args, NULL);
  return NULL;
}

void pair_files2pair_file_buffers_init(pthread_t *pair_files2pair_file_buffers_thread, buffer_t **fid2sorted_pairs, char *OUTPUT_PREFIX, int *num_files, b2g_libraries_t *libraries, int *WRITE_CONCORDANT, int *GASV_PRO) {
  assert(pair_files2pair_file_buffers_thread && fid2sorted_pairs && OUTPUT_PREFIX && num_files && in_range(*num_files, 0, MAX_BAM_FILES) && libraries && WRITE_CONCORDANT && is_bool(*WRITE_CONCORDANT) && GASV_PRO && is_bool(*GASV_PRO));  

  pthread_create(pair_files2pair_file_buffers_thread, NULL, pair_files2pair_file_buffers_start, list(6, fid2sorted_pairs, OUTPUT_PREFIX, num_files, libraries, WRITE_CONCORDANT, GASV_PRO));
}
