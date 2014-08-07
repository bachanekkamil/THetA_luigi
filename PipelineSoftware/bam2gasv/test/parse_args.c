#include <assert.h>
#include <string.h>
#include <stdio.h>
#include "input_args2params.h"
#include "cutoff_lminlmax.h"



void test_main_parser() {
  char *BAM_PATH;
  char *OUTPUT_PREFIX;
  int WRITE_LOWQ = 0;
  int LIB_SEP = 1;
  int MAPPING_QUALITY = 10;
  int WRITE_CONCORDANT = 0;
  int LOW_MEMORY = 0; 
  int AMBIGUOUS = 0;
  int VERBOSE = 0;
  int CUTOFF_LMINLMAX_X, CUTOFF_LMINLMAX_Y;
  cutoff_mode_t CUTOFF_LMINLMAX_MODE;
  char CUTOFF_LMINLMAX_NAME[256];
  int PROPER_LENGTH = 10000;
  int USE_NUMBER_READS = 500000;
  
  // test exact cutoff
  char *argv[4] = {"blah", "bam_path", "-CUTOFF_LMINLMAX", "EXACT=10,30"};    
  assert(!input_args2params(4, argv, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));
  assert(EXACT == CUTOFF_LMINLMAX_MODE);
  assert(10 == CUTOFF_LMINLMAX_X);
  assert(30 == CUTOFF_LMINLMAX_Y);
  assert(0 == WRITE_LOWQ);

  // test filename cutoff
  char *argv2[6] = {"blah", "bam_path", "-CUTOFF_LMINLMAX", "FILE=file_name", "-WRITE_LOWQ", "true"};
  assert(!input_args2params(6, argv2, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));
  assert(FILENAME == CUTOFF_LMINLMAX_MODE);
  assert(!strcmp(CUTOFF_LMINLMAX_NAME, "file_name"));
  assert(1 == WRITE_LOWQ);

  // test unknown arguments
  char *argv3[4] = {"blah", "bam_path", "-UNKNOWN", "true"};
  assert(input_args2params(4, argv3, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));

  // test too few args
  char *argv4[1] = {"blah"};
  assert(input_args2params(1, argv4, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));

  // test invalid bam_path
  char *argv5[3] = {"blah", "-WRITE_LOWQ", "true"};
  assert(input_args2params(3, argv5, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));

  // test unknown values
  char *argv6[4] = {"blah", "bam_path", "-WRITE_LOWQ", "maybe"};
  assert(input_args2params(4, argv6, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));


  // test not enough args
  char *argv7[3] = {"blah", "bam_path", "-WRITE_LOWQ"};
  assert(input_args2params(3, argv7, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));
  
  // test cutoff_lminlmax error handling
  char *argv8[4] = {"blah", "bam_path", "-CUTOFF_LMINLMAX", "PCT =75"};
  assert(input_args2params(4, argv8, 0, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));

  // test that everything reads
  char *argv9[24] = {"blah", "bam_path", "-OUTPUT_PREFIX", "prefix", "-CUTOFF_LMINLMAX", "PCT=75%", "-MAPPING_QUALITY", "25", "-WRITE_CONCORDANT", "true", "-WRITE_LOWQ", "true", "-LOW_MEMORY", "true", "-AMBIG", "true", "-LIBRARY_SEPARATED", "all", "-VERBOSE", "false", "-PROPER_LENGTH", "5000", "-USE_NUMBER_READS", "10000"};
  assert(!input_args2params(24, argv9, 1, &BAM_PATH, &OUTPUT_PREFIX, &MAPPING_QUALITY, &WRITE_CONCORDANT, &WRITE_LOWQ, &LOW_MEMORY, &AMBIGUOUS, &LIB_SEP, &VERBOSE, &CUTOFF_LMINLMAX_X, &CUTOFF_LMINLMAX_Y, &CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_NAME, &PROPER_LENGTH, &USE_NUMBER_READS));
  assert(!strcmp("prefix", OUTPUT_PREFIX));
  assert(PCT == CUTOFF_LMINLMAX_MODE);
  assert(75 == CUTOFF_LMINLMAX_X);
  assert(25 == MAPPING_QUALITY);
  assert(WRITE_CONCORDANT);
  assert(WRITE_LOWQ);
  assert(LOW_MEMORY);
  assert(AMBIGUOUS);
  assert(!LIB_SEP);
  assert(!VERBOSE);
  assert(5000 == PROPER_LENGTH);
  assert(10000 == USE_NUMBER_READS);
  
  
  
}

int main() {

  

  test_main_parser();
 

  return 0;
}
