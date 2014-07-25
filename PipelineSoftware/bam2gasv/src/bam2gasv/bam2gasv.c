#include "bam2gasv.h"
#include "btg_input2params.h"
#include "params2gasv_input.h"


int main(int argc, char **argv) {

  // Set the default parameters,
  char *BAM_PATH;
  char *OUTPUT_PREFIX;
  int WRITE_LOWQ = 0;
  int LIB_SEP = 1;
  int MAPPING_QUALITY = 10;
  int WRITE_CONCORDANT = 0;
  int LOW_MEMORY = 0; 
  int AMBIGUOUS = 0;
  int VERBOSE = 1;
  int USE_NUMBER_READS = 500000;
  int PROPER_LENGTH = 10000;
  b2g_cutoff_lminlmax_mode_t CUTOFF_MODE = PCT;
  int CUTOFF_X = 99;
  int CUTOFF_Y = -1;
  char *CUTOFF_NAME = NULL;
  int DEBUG_LEVEL;
  int WRITE_SPLITREAD = 0;
  int MIN_ALIGNED_PCT = 95;
  char *CHROMOSOME_NAMING = NULL;
  b2g_platform_t PLATFORM = 0;
  int VALIDATION_STRINGENCY = 0;
  int GASV_PRO = 0;
  int IGNORE_DUPLICATES = 1;
  int QNAME_SORTED = 0;
  int SPLIT_BY_CHROMOSOME = 0;
  unsigned int num_skipped = 0;

  b2g_error_t ERR;

  // parse any command line arguments,
  if ((ERR = btg_input2params(argc, argv, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_X, &CUTOFF_Y, &CUTOFF_MODE, &CUTOFF_NAME, &PROPER_LENGTH, &USE_NUMBER_READS, &DEBUG_LEVEL, &WRITE_SPLITREAD, &MIN_ALIGNED_PCT, &CHROMOSOME_NAMING, &PLATFORM, &VALIDATION_STRINGENCY, &GASV_PRO, &IGNORE_DUPLICATES, &QNAME_SORTED, &SPLIT_BY_CHROMOSOME))) return b2g_error_print(ERR);

  // and start the program with the resultant arguments.
  else if ((ERR = params2gasv_input(BAM_PATH, OUTPUT_PREFIX, MAPPING_QUALITY, WRITE_CONCORDANT, WRITE_LOWQ, LOW_MEMORY, AMBIGUOUS, LIB_SEP, VERBOSE, CUTOFF_X, CUTOFF_Y, CUTOFF_MODE, CUTOFF_NAME, PROPER_LENGTH, USE_NUMBER_READS, DEBUG_LEVEL, WRITE_SPLITREAD, MIN_ALIGNED_PCT, CHROMOSOME_NAMING, PLATFORM, VALIDATION_STRINGENCY, GASV_PRO, IGNORE_DUPLICATES, QNAME_SORTED, SPLIT_BY_CHROMOSOME, &num_skipped))) return b2g_error_print(ERR);

  if (VERBOSE) printf("Bam2GASV complete\n");

  return 0;
}
