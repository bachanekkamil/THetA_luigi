#include <assert.h>
#include "bam2gasv.h"
#include "params2gasv_input.h"
#include "b2g_error.h"
#include "b2g_signal.h"

int main(int argc, char **argv) {

  char *BAM_PATH = "test/Example.bam";
  char *OUTPUT_PREFIX = "tmp/integration/lowmem/Example";
  int WRITE_LOWQ = 0;
  int LIB_SEP = 1;
  int MAPPING_QUALITY = 10;
  int WRITE_CONCORDANT = 1;
  int LOW_MEMORY = 1; 
  int AMBIGUOUS = 0;
  int VERBOSE = 0;
  int USE_NUMBER_READS = 249847; // Different count to match different counting strategy between this and old JAVA version.
  int PROPER_LENGTH = 10000;
  int CUTOFF_MODE = FILENAME;
  int CUTOFF_X = -1;
  int CUTOFF_Y = -1;
  char *CUTOFF_NAME = "test/cutoff_lminlmax.txt";
  int DEBUG_LEVEL = 0;
  int WRITE_SPLITREAD = 0;
  int MIN_ALIGNED_PCT = 95;
  char *CHROMOSOME_NAMING = NULL;
  int PLATFORM = 0;
  int VALIDATION_STRINGENCY = 0;
  int GASV_PRO = 1;
  int IGNORE_DUPLICATES = 1;
  int QNAME_SORTED = 0;
  int SPLIT_BY_CHROMOSOME = 0;
  unsigned int num_skipped = 0;

  MAX_BAM_FILES = 3;

  assert(B2GERR_NO_ERROR == params2gasv_input(BAM_PATH, OUTPUT_PREFIX, MAPPING_QUALITY, WRITE_CONCORDANT, WRITE_LOWQ, LOW_MEMORY, AMBIGUOUS, LIB_SEP, VERBOSE, CUTOFF_X, CUTOFF_Y, CUTOFF_MODE, CUTOFF_NAME, PROPER_LENGTH, USE_NUMBER_READS, DEBUG_LEVEL, WRITE_SPLITREAD, MIN_ALIGNED_PCT, CHROMOSOME_NAMING, PLATFORM, VALIDATION_STRINGENCY, GASV_PRO, IGNORE_DUPLICATES, QNAME_SORTED, SPLIT_BY_CHROMOSOME, &num_skipped));

  return 0;
}
