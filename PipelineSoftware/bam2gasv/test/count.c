#include <assert.h>
#include "bam.h"
#include "sam.h"

int main(int argc, char **argv) {
  assert(3 == argc);
  
  bam1_t *read = bam_init1();
  samfile_t *bamfile = samopen(argv[1], "rb", 0);
  
  int count = 0;
  while (-1 < samread(bamfile, read)) count++;
  
  assert(count == atoi(argv[2]));

  samclose(bamfile);
  bam_destroy1(read);
  return 0;
}
