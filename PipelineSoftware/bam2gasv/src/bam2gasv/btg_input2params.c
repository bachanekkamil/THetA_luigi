#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include "b2g_constants.h"
#include "btg_input2params.h"
#include "b2g_cutoff_lminlmax.h"
#include "b2g_error.h"

// Utility parsers

static int _parse_boolean(char *arg, int *option) {
  if (!strcmp(arg, "false")) *option = 0;
  else if (!strcmp(arg, "true")) *option = 1;
  else return 1;
  return 0;
}

static int _parse_range(char *arg, int *option, int low, int high) {
  return 1 != sscanf(arg, "%d", option) || !(low <= *option && *option <= high);
}

static int _parse_platform(char *arg, b2g_platform_t *PLATFORM) {
  if (!strcasecmp(arg, "illumina")) *PLATFORM = ILLUMINA;
  else if (!strcasecmp(arg, "solid")) *PLATFORM = SOLID;
  else if (!strcasecmp(arg, "matepair")) *PLATFORM = MATEPAIR;
  else return 1;
  return 0;
}

static int _parse_library_separated(char *arg, int *LIBRARY_SEPARATED) {
  if (!strcasecmp(arg, "all")) *LIBRARY_SEPARATED = 0;
  else if (!strcasecmp(arg, "sep")) *LIBRARY_SEPARATED = 1;
  else return 1;
  return 0;
}

static int _parse_lminlmax_mode(char *s, b2g_cutoff_lminlmax_mode_t *mode, int *X, int *Y, char **name) {
  if (1 == sscanf(s, "SD=%d",  X) && 0 <= *X) *mode = SD;
  else if (1 == sscanf(s, "PCT=%d%%", X) && 0 <= *X && *X <= 100) *mode = PCT;
  else if (2 == sscanf(s, "EXACT=%d,%d", X, Y) && 0 <= *X && *X <= *Y) *mode = EXACT;
  else if (!strncmp(s, "FILE=%s", 5)) {
    *mode = FILENAME;  
    *name = &s[5];
  }
  else return 1;
  return 0;
}



b2g_error_t btg_input2params(int argc, char** argv, char** BAM_PATH, char** OUTPUT_PREFIX, int* MAPPING_QUALITY, int* WRITE_CONCORDANT, int* WRITE_LOWQ,  int* LOW_MEMORY, int* AMBIGUOUS, int *LIBRARY_SEPARATED, int *VERBOSE, int *CUTOFF_LMINLMAX_X, int *CUTOFF_LMINLMAX_Y, b2g_cutoff_lminlmax_mode_t *CUTOFF_LMINLMAX_MODE, char **CUTOFF_LMINLMAX_NAME, int *PROPER_LENGTH, int *USE_NUMBER_READS, int *DEBUG_LEVEL, int *WRITE_SPLITREAD, int *MIN_ALIGNED_PCT, char **CHROMOSOME_NAMING, b2g_platform_t *PLATFORM, int *VALIDATION_STRINGENCY, int *GASV_PRO, int *IGNORE_DUPLICATES, int *QNAME_SORTED, int *SPLIT_BY_CHROMOSOME) {
  
  // Set the defaults,
  *BAM_PATH = argv[1];
  *OUTPUT_PREFIX = *BAM_PATH; 
  char *CUTOFF_LMINLMAX_STR = "PCT=99%";

  // validate total input,
  if (argc < 2) return B2GERR_NO_BAM_FILE_SUPPLIED;
  else if (argc % 2 != 0) return B2GERR_OPTION_WITHOUT_ARGUMENT; 
    
  // and then parse individual aptions.
  int i;
  for (i=2; i<argc; i+=2) {

    char *option = argv[i];
    char *arg = argv[i+1];
    
    if (!strcmp(option, "-OUTPUT_PREFIX")) {
      *OUTPUT_PREFIX = arg;
    }
    else if (!strcmp(option, "-CHROMOSOME_NAMING")) {
      *CHROMOSOME_NAMING = arg;
    }
    else if (!strcmp(option, "-CUTOFF_LMINLMAX")) {
      CUTOFF_LMINLMAX_STR = arg;
      if (_parse_lminlmax_mode(CUTOFF_LMINLMAX_STR, CUTOFF_LMINLMAX_MODE, CUTOFF_LMINLMAX_X, CUTOFF_LMINLMAX_Y, CUTOFF_LMINLMAX_NAME)) return B2GERR_CUTOFF_LMINLMAX;
    }
    else if (!strcmp(option, "-MAPPING_QUALITY")) {
      if (_parse_range(arg, MAPPING_QUALITY, 0, 255)) return B2GERR_MAPPING_QUALITY;
    }
    else if (!strcmp(option, "-MIN_ALIGNED_PCT")) {
      if (_parse_range(arg, MIN_ALIGNED_PCT, 50, 100)) return B2GERR_MIN_ALIGNED_PCT;
    }
    else if (!strcmp(option, "-WRITE_CONCORDANT")) {
      if (_parse_boolean(arg, WRITE_CONCORDANT)) return B2GERR_WRITE_CONCORDANT;
    }
    else if (!strcmp(option, "-WRITE_LOWQ")) {      
      if (_parse_boolean(arg, WRITE_LOWQ)) return B2GERR_WRITE_LOWQ;
    }
    else if (!strcmp(option, "-QNAME_SORTED")) {      
      if (_parse_boolean(arg, QNAME_SORTED)) return B2GERR_QNAME_SORTED;
    }
    else if (!strcmp(option, "-SPLIT_BY_CHROMOSOME")) {      
      if (_parse_boolean(arg, SPLIT_BY_CHROMOSOME)) return B2GERR_SPLIT_BY_CHROMOSOME;
    }
    else if (!strcmp(option, "-WRITE_SPLITREAD")) {      
      if (_parse_boolean(arg, WRITE_SPLITREAD)) return B2GERR_WRITE_SPLITREAD;
    }
    else if (!strcmp(option, "-GASVPRO")) {      
      if (_parse_boolean(arg, GASV_PRO)) return B2GERR_GASV_PRO;
    }
    else if (!strcmp(option, "-LARGE_BAM")) {
      if (_parse_boolean(arg, LOW_MEMORY)) return B2GERR_LARGE_BAM;
    }
    else if (!strcmp(option, "-AMBIG")) {
      if (_parse_boolean(arg, AMBIGUOUS)) return B2GERR_AMBIGUOUS;
    }
    else if (!strcmp(option, "-LIBRARY_SEPARATED")) {
      if (_parse_library_separated(arg, LIBRARY_SEPARATED)) return B2GERR_LIBRARY_SEPARATED;
    }
    else if (!strcmp(option, "-VERBOSE")) {
      if (_parse_boolean(arg, VERBOSE)) return B2GERR_VERBOSE;
    }
    else if (!strcmp(option, "-PROPER_LENGTH")) {
      if (_parse_range(arg, PROPER_LENGTH, 0, INT_MAX)) return B2GERR_PROPER_LENGTH;
    }
    else if (!strcmp(option, "-PLATFORM")) {
      if (_parse_platform(arg, PLATFORM)) return B2GERR_PLATFORM;
    }
    else if (!strcmp(option, "-USE_NUMBER_READS")) {
      if (_parse_range(arg, USE_NUMBER_READS, 1, INT_MAX)) return B2GERR_USE_NUMBER_READS;
    }
    else if (!strcmp(option, "-DEBUG_LEVEL")) {
      if (_parse_range(arg, DEBUG_LEVEL, 0, 3)) return B2GERR_DEBUG_LEVEL;
    }
    else return B2GERR_UNKNOWN_OPTION;      
  }
  

  if (*VERBOSE) {
    printf("BAM_PATH:\t\t%s\n", *BAM_PATH);
    printf("\n==========Output Options==========\n");
    printf("OUTPUT_PREFIX:\t\t%s\n", *OUTPUT_PREFIX);
    printf("WRITE_CONCORDANT:\t%s\n", BOOLS[*WRITE_CONCORDANT]);
    printf("AMBIG:\t\t\t%s\n", BOOLS[*AMBIGUOUS]);
    printf("WRITE_LOWQ:\t\t%s\n", BOOLS[*WRITE_LOWQ]);  
    printf("WRITE_SPLITREAD:\t%s\n", BOOLS[*WRITE_SPLITREAD]);
    printf("GASVPRO:\t\t%s\n", BOOLS[*GASV_PRO]);      
    printf("MAPPING_QUALITY:\t%d\n", *MAPPING_QUALITY);
    printf("MIN_ALIGNED_PCT:\t%d\n", *MIN_ALIGNED_PCT);
    printf("SPLIT_BY_CHROMOSOME:\t%s\n", BOOLS[*SPLIT_BY_CHROMOSOME]);
    printf("\n==========Library Options==========\n");
    printf("LIBRARY_SEPARATED:\t%s\n", BOOLS[*LIBRARY_SEPARATED]);
    printf("CUTOFF_LMINLMAX:\t%s\n", CUTOFF_LMINLMAX_STR);    
    printf("PROPER_LENGTH:\t\t%d\n", *PROPER_LENGTH);
    printf("USE_NUMBER_READS:\t%d\n", *USE_NUMBER_READS);
    printf("PLATFORM:\t\t%s\n", PLATFORMS[*PLATFORM]);
    printf("\n==========Optimization Options==========\n");    
    printf("LARGE_BAM:\t\t%s\n", BOOLS[*LOW_MEMORY]);
    printf("QNAME_SORTED:\t\t%s\n", BOOLS[*QNAME_SORTED]);
    printf("\n==========Reporting Options==========\n");
    printf("VERBOSE:\t\t%s\n", BOOLS[*VERBOSE]);
    printf("\n\n");
  }

  return B2GERR_NO_ERROR;
}


