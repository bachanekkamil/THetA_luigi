#include "bam.h"
#include "sam.h"
#include <assert.h>
#include <time.h>

int main(int argc, char **argv) {
  assert(2 == argc);
  
  time_t start = time(NULL);
  samfile_t *in = samopen(argv[1], "rb", 0);
  bam1_t *b = bam_init1();
  while (-1 < samread(in, b)) {}
  bam_destroy1(b);
  samclose(in);
  printf("%f", difftime(time(NULL), start));

  return 0;
}
