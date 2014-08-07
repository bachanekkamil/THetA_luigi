#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "gasv_pair.h"
#include "variant_type.h"
#include "pairs2gasvfiles.h"


int main(int argc, char** argv) {

  gasv_pair_t *p1 = gasv_pair("IL5_582:8:121:538:973", 0, 554493, 554528, 1, 0, 554494, 554529, 0);
  gasv_pair_t *p2 = gasv_pair("IL10_578:7:22:80:581", 0, 554493, 554528, 1, 0, 554496, 554531, 0);
  gasv_pair_t *p3 = gasv_pair("IL5_582:6:144:342:88", 0, 754566, 754601, 1, 0, 754567, 754602, 0);

  list_t *goal = list(3, p1, p2, p3);
  list_t *pairs = list(6, &bam_ERR, &bam_EOAF, &bam_EOF, p3, p1, &bam_EOF, p2);
  list_t *files = list(0);
  
  pairs2gasv(DIV, pairs,  files);

  //merge_unsorted_pairs(DIV, pairs, unsorted, NULL);
  
  //assert(!list_cmp(goal, sorted, NULL));
  list_free(sorted, free);
  list_free(goal, NULL);

  return 0;
}
