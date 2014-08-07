#include <assert.h>
#include "bam.h"
#include "sam.h"

int main(int argc, char **argv) {
  if (4 != argc) {
    printf("Usage: head <file.in> <file.out> <num-reads>\n\n");
    return 0;
  }
  
  bam1_t *read = bam_init1();
  samfile_t *in = samopen(argv[1], "rb", 0);
  samfile_t *out = samopen(argv[2], "wbh", in->header);
  int count = atoi(argv[3]);  
  assert(in && out && 0 <= count);

  

  while (count && -1 < samread(in, read)) {
    samwrite(out, read);
    count--;
  }
  

  samclose(in);
  samclose(out);
  bam_destroy1(read);
  return 0;
}
