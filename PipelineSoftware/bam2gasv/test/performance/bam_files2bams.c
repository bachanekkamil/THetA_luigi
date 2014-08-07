#include "bam_files2bams.h"
#include "buffer.h"
#include "bam.h"
#include "bam_utils.h"
#include <time.h>

int main(int argc, char **argv) {

  time_t start = time(NULL);
  buffer_t *bams = buffer(1000, sizeof(bam1_t));
  bam_files2bams(argv[1], NULL, 1, 0, bams, 0, 1);
  buffer_free(bams, bam_clear);
  printf("%f\n", difftime(time(NULL), start));

  return 0;
}
