#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "gasv_pair.h"
#include "variant_type.h"
#include "pairs2gasv_file.h"
#include "pairs2sorted_pairs.h"
#include "bam_buffer.h"
#include "list.h"


int main(int argc, char** argv) {

  gasv_pair_t *p1 = gasv_pair("IL5_582:8:121:538:973", 0, 554493, 554528, 1, 0, 554494, 554529, 0);
  gasv_pair_t *p2 = gasv_pair("IL10_578:7:22:80:581", 0, 554493, 554528, 1, 0, 554496, 554531, 0);
  gasv_pair_t *p3 = gasv_pair("IL5_582:6:144:342:88", 0, 754566, 754601, 1, 0, 754567, 754602, 0);
  
  list_t *pairs = list(6, &bam_EOAF, &bam_EOF, p3, p1, &bam_EOF, p2);
  list_t *file1 = list(1, p2);
  list_t *file2 = list(2, p1, p3);

  buffer_t *gfiles = buffer(10, sizeof(list_t));

  
  pairs2sorted_pairs(TRANS, pairs,  gfiles);  

  assert(!list_cmp(buffer_peek(gfiles), file1, NULL));
  list_clear(buffer_peek(gfiles), NULL);
  buffer_consume(gfiles);
  assert(!list_cmp(buffer_peek(gfiles), file2, NULL));
  list_clear(buffer_peek(gfiles), NULL);
  buffer_consume(gfiles);
  

  list_t *pairs1 = list(3, p1, &bam_EOF, p2);
  list_t *pairs2 = list(3,  &bam_EOAF, &bam_EOF, p3);  

  pairs2sorted_pairs(INV, pairs1, gfiles);
  pairs2sorted_pairs(INV, pairs2, gfiles);

  assert(!list_cmp(buffer_peek(gfiles), file1, NULL));
  list_clear(buffer_peek(gfiles), NULL);
  buffer_consume(gfiles);
  assert(!list_cmp(buffer_peek(gfiles), file2, NULL));
  list_clear(buffer_peek(gfiles), NULL);
  
  list_free(file1, free);
  list_free(file2, free);
  buffer_free(gfiles, NULL);  

  return 0;
}

