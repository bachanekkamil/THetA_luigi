#include <assert.h>
#include "test_utils.h"
#include "list.h"
#include "b2g_bam.h"
#include "sam.h"

list_t *bam_path2bams(char *BAM_PATH, int n) {
  assert(0 < n);
  int i;
  samfile_t *in = b2g_samfile_open(BAM_PATH, "rb", 0);
  list_t *bams = NULL;
  bam1_t *b = bam_init1();
  for (i=0; i<n; i++) {
    if (-1 < samread(in, b)) break;
    bams = cons(b, bams);
    b = bam_init1();
  }
  bam_destroy1(b);
  samclose(in);
  return list_reverse(bams);
}

bam1_t *bam_path2bam(char *BAM_PATH) {
  samfile_t *in = b2g_samfile_open(BAM_PATH, "rb", 0);
  bam1_t *b = bam_init1();
  assert(-1 < samread(in, b));
  samclose(in);
  return b;
}

