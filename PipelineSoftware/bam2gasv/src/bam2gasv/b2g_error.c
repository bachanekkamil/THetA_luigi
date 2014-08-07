

#include <stdio.h>
#include <stdlib.h>
#include "b2g_error.h"

b2g_error_t B2GERRNO = B2GERR_NO_ERROR;

char *ERRMSG_USAGE = "USAGE: bam2gasv <bam filepath or hyperlink> [Options]";

char *ERRMSG_BAD_INPUT = "ERROR while parsing BamToGASV option";

char *ERRMSG_OUTPUT_PREFIX = "-OUTPUT_PREFIX [String] (Default: BAM filename)\n\tDefines the directory and filename prefix for your output files (Ex: path/to/output/Example).";

char *ERRMSG_MAPPING_QUALITY =  "-MAPPING_QUALITY [0-100] (Default: 10)\n\tReads with quality less than MAPPING_QUALITY are low quality.";

char *ERRMSG_WRITE_CONCORDANT =  "-WRITE_CONCORDANT [true|false] (Default: false)\n\tIf true, writes concordant reads to a file. (Warning - this will be large!)";

char *ERRMSG_WRITE_LOWQ =  "-WRITE_LOWQ [true|false] (Default: false)\n\tIf true, writes low quality reads to a file. (Warning - this will be large!)";

char *ERRMSG_QNAME_SORTED =  "-QNAME_SORTED [true|false] (Default: false)\n\tIf true, assumes that the BAM is sorted by QNAME, allowing greater speed and memory conservation. This option is automatically enabled for AMBIGUOUS mode.";

char *ERRMSG_SPLIT_BY_CHROMOSOME =  "-SPLIT_BY_CHROMOSOME [true|false] (Default: false)\n\tIf true, splits the final output into separate files based on the chromosomes of the pairs.";

char *ERRMSG_LARGE_BAM =  "-LARGE_BAM [true|false] (Default: false)\n\tIf true, BAM2GASV will run more slowly, but can handle BAM files much larger than the available memory.";

char *ERRMSG_AMBIGUOUS = "-AMBIG [true|false] (Default: false)\n\tIf true, all reads with the same template name will be paired with one another round-robin such that all reads with the \"first segment in the template\" flag set will be paired with all reads with the \"second segment in the template\" flag set. (as opposed to assuming 2 reads per template name). This mode requires that the BAM file be sorted by qname, and automatically enables -QNAME_SORTED.";

char *ERRMSG_LIBRARY_SEPARATED =  "-LIBRARY_SEPARATED [String [all|sep]] Default: sep\n\tWhen sep, treats each library in the BAM file as a separate input, resulting in one set of GASV files per library.";

char *ERRMSG_VERBOSE = "-VERBOSE [true|false] (Default: true)\n\tIf true, prints arguments and progress information during execution.";

char *ERRMSG_CUTOFF_LMINLMAX =  "-CUTOFF_LMINLMAX [String] (Default: PCT=99%)\n\tSpecifies exact upper and lower bounds on the fragment distribution, or the method by which to estimate those bounds empirically from the bam file. The fragment distribution bounds determine which pairs are concordant, insertions, or deletions.\n\tPCT=X%  Take the quantile on the top/bottom X percent.\n\tSD=X    Take the standard deviation above/below the mean.\n\tEXACT=X,Y       Set Lmin to X and Lmax to Y.\n\tFILE=fname      File of the form '<libname>\t<CUTOFF_LMINLMAX>' for using different cutoffs on different libraries.";

char *ERRMSG_PROPER_LENGTH =  "-PROPER_LENGTH [Integer] (Default: 10000)\n\tIgnore ESPs with separation larger than PROPER_LENGTH when estimating Lmin and Lmax from the bam file. If PROPER_LENGTH is 0, it will be ignored.";

char *ERRMSG_PLATFORM = "-PLATFORM [Illumina|SOLiD|MatePair] (Default: Illumina)\n\tIllumina\tReads are sequenced with the Illumina platform (paired-read orientation --> <--).\n\tSOLiD\tReads are sequenced with the SOLiD platform.\n\tMatePair\tReads are sequenced with the mate pair orientation (outward orientation <-- -->).";

char *ERRMSG_USE_NUMBER_READS =  "-USE_NUMBER_READS [Integer] (Default: 500000)\n\tThe number of fragments in the BAM file to use in estimating Lmax and Lmin.  To get a good estimate of the distribution, up to 1 million fragments is sufficient for most applicaitons.";

char *ERRMSG_DEBUG_LEVEL =  "-DEBUG_LEVEL [Integer] (Default: 0) 0 is no debugging. Otherwise, enable DEBUG_LEVEL consecutive steps in the processing pipeline, cancelling all phases after the DEBUG_LEVEL'th step.";

char *ERRMSG_WRITE_SPLITREAD =  "-WRITE_SPLITREAD [true|false] (Default: false)\n\tIf true, writes candidate split reads to a file (Warning - this will be large!)";

char *ERRMSG_MIN_ALIGNED_PCT =  "-MIN_ALIGNED_PCT [50-100] (Default: 95)\n\tReads with alignment less than MIN_ALIGNED_PCT percent are low quality";

char *ERRMSG_GASV_PRO = "-GASVPRO [true|false] (Default: false)\n\tIf true, generates GASVPro parameters file and concordant file. Warning - this will be large!)";


int print_help() {
  printf("%s\n", ERRMSG_USAGE);
  printf("\n==========Output Options==========\n");
  printf("%s\n", ERRMSG_OUTPUT_PREFIX);
  printf("%s\n", ERRMSG_WRITE_CONCORDANT);
  printf("%s\n", ERRMSG_AMBIGUOUS);
  printf("%s\n", ERRMSG_WRITE_LOWQ);
  printf("%s\n", ERRMSG_WRITE_SPLITREAD);
  printf("%s\n", ERRMSG_GASV_PRO);
  printf("%s\n", ERRMSG_MAPPING_QUALITY);
  printf("%s\n", ERRMSG_MIN_ALIGNED_PCT);
  printf("%s\n", ERRMSG_SPLIT_BY_CHROMOSOME);
  printf("\n==========Library Options==========\n");
  printf("%s\n", ERRMSG_LIBRARY_SEPARATED);
  printf("%s\n", ERRMSG_CUTOFF_LMINLMAX);
  printf("%s\n", ERRMSG_PROPER_LENGTH);
  printf("%s\n", ERRMSG_USE_NUMBER_READS);
  printf("%s\n", ERRMSG_PLATFORM);
  printf("\n==========Optimization Options==========\n");
  printf("%s\n", ERRMSG_LARGE_BAM);
  printf("%s\n", ERRMSG_QNAME_SORTED);
  printf("\n==========Reporting Options==========\n");
  printf("%s\n", ERRMSG_VERBOSE);
  return 0;
}



void *b2g_malloc(unsigned int size) {
  void *mem = malloc(size);
  if (!mem) b2g_exit(B2GERR_OUT_OF_MEMORY);
  return mem;
}

// Ideally all errors would propagate back up the stack and exit gracefully. This would require tweaks to the thread topology. For the moment, we should prefer informative crashing to uninformative crashing.
void b2g_exit(b2g_error_t err) {
  b2g_error_print(err);
  exit(-1);
}

int b2g_error_print(b2g_error_t err) {

  switch (err) {

  case B2GERR_NO_ERROR: return 0;

  case B2GERR_NO_BAM_FILE_SUPPLIED:
    return print_help();
    
  case B2GERR_OUTPUT_PREFIX:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_OUTPUT_PREFIX);
    break;

  case B2GERR_MAPPING_QUALITY:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_MAPPING_QUALITY);
    break;
    
  case B2GERR_WRITE_CONCORDANT:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_WRITE_CONCORDANT);
    break;

  case B2GERR_WRITE_LOWQ:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_WRITE_LOWQ);
    break;

  case B2GERR_LARGE_BAM:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_LARGE_BAM);
    break;

  case B2GERR_AMBIGUOUS:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_AMBIGUOUS);
    break;

  case B2GERR_LIBRARY_SEPARATED:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_LIBRARY_SEPARATED);
    break;

  case B2GERR_VERBOSE:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_VERBOSE);
    break;

  case B2GERR_CUTOFF_LMINLMAX:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_CUTOFF_LMINLMAX);
    break;

  case B2GERR_PROPER_LENGTH:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_PROPER_LENGTH);
    break;

  case B2GERR_PLATFORM:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_PLATFORM);
    break;

  case B2GERR_USE_NUMBER_READS:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_USE_NUMBER_READS);
    break;    

  case B2GERR_DEBUG_LEVEL:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_DEBUG_LEVEL);
    break;

  case B2GERR_WRITE_SPLITREAD:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_WRITE_SPLITREAD);
    break;

  case B2GERR_MIN_ALIGNED_PCT:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_MIN_ALIGNED_PCT);
    break;

  case B2GERR_GASV_PRO:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_GASV_PRO);
    break;

  case B2GERR_QNAME_SORTED:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_WRITE_LOWQ);
    break;

  case B2GERR_SPLIT_BY_CHROMOSOME:
    printf("%s:\n", ERRMSG_BAD_INPUT);
    printf(ERRMSG_SPLIT_BY_CHROMOSOME);
    break;
    
    case B2GERR_UNKNOWN_OPTION:
      printf("ERROR: Unrecognized option: make sure your option name is upper case, contains no spaces, and starts with a single dash, ie: -WRITE_LOWQ\n");
    break;

  case B2GERR_OPTION_WITHOUT_ARGUMENT:
    printf("ERROR: Wrong number of arguments: \nMake sure you include both the path to the BAM file and exactly one argument for every additional option, ie: bam2gasv <path/to/bamfile> -WRITE_LOWQ true\n");
    break;

  case B2GERR_NO_BAM_FILE_FOUND:
    printf("ERROR: Either no BAM file was found at the file path provided as the first argument to bam2gasv, or a BAM file was found, but incorrectly formatted (ie no header).\n");
    break;

  case B2GERR_NO_CHROMOSOME_NAMING_FILE_FOUND:
    printf("ERROR: No CHROMOSOME_NAMING file was found at the file path provided to the -CHROMOSOME_NAMING option.\n");
    break;    


  case B2GERR_CHROMOSOME_NAMING_INVALID_FORMAT:
    printf("ERROR: The CHROMOSOME_NAMING file supplied with -CHROMOSOME_NAMING is formatted incorrectly. Please ensure that the file contains a series of lines of the form <BAM file chromosome name>\\t<Integer ID><\n");
    break;        
    
  case B2GERR_NO_CUTOFF_FILE_FOUND:
    printf("ERROR: Unable to open file specified with -CUTOFF_LMINLMAX. Please check that the file is at the path supplied and has appropriate permissions.\n");
    break;        
  
 case B2GERR_CUTOFF_FILE_INVALID_FORMAT:
    printf("ERROR: Unable to parse the file specified by -CUTOFF_LMINLMAX FILENAME=<filename>. Please ensure that this file is of the form <library name>\\t<CUTOFF_LMINLMAX>\n");
    break;        
    
  case B2GERR_LIBRARY_NOT_FOUND:
    printf("ERROR: One of the libraries you have supplied via -CUTOFF_LMINLMAX FILE=<filename> was not found in the BAM header. Please ensure that your BAM file's header lists the appropriate library, and that you have spelled the name correctly in your file.\n");
    break;        

  case B2GERR_OUTPUT_DIR_NO_WRITE:
    printf("ERROR: Unable to write to specified output directory. Please ensure that the directory portion of -OUTPUT_PREFIX exists and is writeable.\n");
    break;
  
  case B2GERR_UNSORTED:
    printf("ERROR: You have enabled -QNAME_SORTED but the BAM file is not in qname sorted order.\n");
    break;
    
  case B2GERR_OUT_OF_MEMORY:
    printf("ERROR: Out of memory. Try enabling -LARGE_BAM to run in a more memory-conservative mode.\n");
    break;
        
  case B2G_ERR_CANNOT_OPEN_OUTPUT_FILES:
    printf("ERROR: There was a problem opening the final output files. Please check the permissions of the output directory.\n");
    break;

  case B2GERR_WOULD_OVERWRITE_PREVIOUS:
    printf("ERROR: Output with specified OUTPUT_PREFIX already exists. Please move the existing output to avoid overwriting it.\n");
    break;
    


  }  

  return err;
}


