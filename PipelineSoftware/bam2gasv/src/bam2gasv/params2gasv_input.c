#include <assert.h>
#include <stdio.h>
#include <sys/stat.h>
#include "params2gasv_input.h"
#include "b2g_cutoff_lminlmax.h"
#include "params2bam_files.h"
#include "b2g_bam.h"
#include "bam_files2pair_files.h"
#include "buffer.h"
#include "list.h"
#include "b2g_chromosome.h"
#include "pair_files2gasv_input.h"
#include "bam.h"
#include "b2g_libraries.h"
#include "b2g_bam_header.h"
#include "io.h"

// Releases all resources and passes the error through for easy chaining with return.
static b2g_error_t _cleanup(samfile_t *in, FILE *chromosome_naming_file, FILE *cutoff_file, samfile_t *lowq_file, samfile_t *split_file, FILE *info_file, FILE *gasv_file, FILE *gasvpro_file, hash_t *pairtable, b2g_error_t ret) {
  if (in) samclose(in);
  if (chromosome_naming_file) fclose(chromosome_naming_file);
  if (cutoff_file) fclose(cutoff_file);
  if (lowq_file) samclose(lowq_file);
  if (split_file) samclose(split_file);
  if (info_file) fclose(info_file);
  if (gasv_file) fclose(gasv_file);
  if (gasvpro_file) fclose(gasvpro_file);
  if (pairtable) hash_free(pairtable, NULL);
  return ret;
}

// Open all external resources.
static b2g_error_t _open_files(samfile_t **in, FILE **chromosome_naming_file, FILE **cutoff_file, samfile_t **lowq_file, samfile_t **split_file, FILE **info_file, FILE **gasv_file, FILE **gasvpro_file, char *BAM_PATH, char *OUTPUT_PREFIX, char *CHROMOSOME_NAMING, b2g_cutoff_lminlmax_mode_t CUTOFF_MODE, char *CUTOFF_NAME, int WRITE_LOWQ, int WRITE_SPLITREAD, int GASV_PRO) {
  char info_filename[MAX_FILENAME_SIZE];
  sprintf(info_filename, "%s.info", OUTPUT_PREFIX);
  if (!access(info_filename, F_OK)) return B2GERR_WOULD_OVERWRITE_PREVIOUS;

  if (!(*in = b2g_samfile_open(BAM_PATH, "rb", 0))) return B2GERR_NO_BAM_FILE_FOUND;
  if (CHROMOSOME_NAMING && !(*chromosome_naming_file = fopen(CHROMOSOME_NAMING, "r"))) return B2GERR_NO_CHROMOSOME_NAMING_FILE_FOUND;
  if (FILENAME == CUTOFF_MODE && !(*cutoff_file = fopen(CUTOFF_NAME, "r"))) return B2GERR_NO_CUTOFF_FILE_FOUND;
  if ((WRITE_LOWQ) && !(*lowq_file = b2g_samfile_open("%s_lowqual.bam", "wbh", (*in)->header, OUTPUT_PREFIX))) return B2GERR_OUTPUT_DIR_NO_WRITE;
  if (WRITE_SPLITREAD && !(*split_file = b2g_samfile_open("%s_splitread.bam", "wbh", (*in)->header, OUTPUT_PREFIX))) return B2GERR_OUTPUT_DIR_NO_WRITE;
  if (!(*info_file = file_open("%s.info", "w", OUTPUT_PREFIX))) return B2GERR_OUTPUT_DIR_NO_WRITE;
  if (!(*gasv_file = file_open("%s.gasv.in", "w", OUTPUT_PREFIX))) return B2GERR_OUTPUT_DIR_NO_WRITE;
  if (GASV_PRO && !(*gasvpro_file = file_open("%s.gasvpro.in", "w", OUTPUT_PREFIX))) return B2GERR_OUTPUT_DIR_NO_WRITE;
  return B2GERR_NO_ERROR;
}

// Primary execution logic for this component..
static b2g_error_t _split_sort_merge_dump(char *BAM_PATH, char *OUTPUT_PREFIX, int MAPPING_QUALITY, int WRITE_CONCORDANT, int WRITE_LOWQ, int LOW_MEMORY, int AMBIGUOUS, int LIB_SEP, int VERBOSE, int CUTOFF_X, int CUTOFF_Y, b2g_cutoff_lminlmax_mode_t CUTOFF_MODE, char *CUTOFF_NAME, int PROPER_LENGTH, int USE_NUMBER_READS, int DEBUG_LEVEL, int WRITE_SPLITREAD, int MIN_ALIGNED_PCT, char *CHROMOSOME_NAMING, b2g_platform_t PLATFORM, int GASV_PRO, int IGNORE_DUPLICATES, samfile_t *lowq_file, samfile_t *split_file, b2g_libraries_t *libraries, b2g_chromosomes_t *chromosomes, hash_t *unpaired, FILE *info_file, FILE *gasv_file, FILE *gasvpro_file, unsigned long GENOME_LENGTH, int QNAME_SORTED, int SPLIT_BY_CHROMOSOME, unsigned int *num_skipped) {
  assert(lowq_file || !WRITE_LOWQ);
  assert(split_file || !WRITE_SPLITREAD);
  int i, some_empty_libraries = 0;
  b2g_error_t ERR = B2GERR_NO_ERROR;

  // Split the BAM file into as many sub-BAMs as needed,
  if (VERBOSE && LOW_MEMORY) printf("Splitting large BAM file into smaller BAM files\n\n");
  int num_files = LOW_MEMORY ? params2bam_files(BAM_PATH, OUTPUT_PREFIX, WRITE_LOWQ, WRITE_SPLITREAD, MAPPING_QUALITY, VERBOSE, chromosomes) : 1;
  assert(num_files <= MAX_BAM_FILES);  

  // pair and sort each sub BAM file,
  if (VERBOSE) printf("Converting BAM file%s %s\n\n", LOW_MEMORY ? "s" : "", LOW_MEMORY ? "into pair files" : "directly into gasv input files");
  if ((ERR = bam_files2pair_files(num_files, BAM_PATH, OUTPUT_PREFIX, libraries, chromosomes, unpaired, lowq_file, split_file, WRITE_LOWQ, MAPPING_QUALITY, LOW_MEMORY, PROPER_LENGTH, AMBIGUOUS, VERBOSE, WRITE_CONCORDANT, DEBUG_LEVEL, WRITE_SPLITREAD, MIN_ALIGNED_PCT, IGNORE_DUPLICATES, PLATFORM, USE_NUMBER_READS, GASV_PRO, QNAME_SORTED, SPLIT_BY_CHROMOSOME, num_skipped))) return ERR;

  // And merge the sorted gasv files back together.
  if (VERBOSE && LOW_MEMORY) printf("Merging pair files into gasv input\n\n");
  if (LOW_MEMORY) pair_files2gasv_input(OUTPUT_PREFIX, num_files, libraries, chromosomes, WRITE_CONCORDANT, GASV_PRO, SPLIT_BY_CHROMOSOME);

  // Dump statistics and gasvpro parameters.
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    if (VERBOSE && !b2g_library_total_concordant(b2g_libraries_get(libraries, i))) printf("%s%s", some_empty_libraries++ ? ", " : "WARNING: no matching pairs found for the following libraries: ", b2g_library_name(b2g_libraries_get(libraries, i)));
  }
  if (some_empty_libraries) printf("\n\n");

  b2g_libraries_write_info(libraries, info_file, USE_NUMBER_READS, VERBOSE);
  b2g_libraries_write_gasv_in(libraries, gasv_file, OUTPUT_PREFIX);  
  if (GASV_PRO) b2g_libraries_write_gasvpro_in(libraries, gasvpro_file, OUTPUT_PREFIX, USE_NUMBER_READS, GENOME_LENGTH);


  return B2GERR_NO_ERROR;
}



// num_skipped counts the number of reads that do not show up in output. Used by validate to account for all the input reads.
b2g_error_t params2gasv_input(char *BAM_PATH, char *OUTPUT_PREFIX, int MAPPING_QUALITY, int WRITE_CONCORDANT, int WRITE_LOWQ, int LOW_MEMORY, int AMBIGUOUS, int LIB_SEP, int VERBOSE, int CUTOFF_X, int CUTOFF_Y, b2g_cutoff_lminlmax_mode_t CUTOFF_MODE, char *CUTOFF_NAME, int PROPER_LENGTH, int USE_NUMBER_READS, int DEBUG_LEVEL, int WRITE_SPLITREAD, int MIN_ALIGNED_PCT, char *CHROMOSOME_NAMING, b2g_platform_t PLATFORM, int VALIDATION_STRINGENCY, int GASV_PRO, int IGNORE_DUPLICATES, int QNAME_SORTED, int SPLIT_BY_CHROMOSOME, unsigned int *num_skipped) {
  assert(BAM_PATH && OUTPUT_PREFIX && in_range(MAPPING_QUALITY, 0, 255) && is_bool(WRITE_CONCORDANT) && is_bool(WRITE_LOWQ) && is_bool(LOW_MEMORY) && is_bool(AMBIGUOUS) && is_bool(LIB_SEP) && is_bool(VERBOSE) && CUTOFF_MODE && (FILENAME != CUTOFF_MODE || CUTOFF_NAME) && is_nonnegative(PROPER_LENGTH) && is_positive(USE_NUMBER_READS) && is_bool(WRITE_SPLITREAD) && in_range(MIN_ALIGNED_PCT, 50, 100) && is_bool(GASV_PRO) && is_bool(IGNORE_DUPLICATES) && is_bool(SPLIT_BY_CHROMOSOME));

  b2g_error_t ERR = B2GERR_NO_ERROR;
  hash_t *pairtable = NULL;
  samfile_t *in = NULL, *lowq_file = NULL, *split_file = NULL;
  FILE *chromosome_naming_file = NULL, *cutoff_file = NULL, *info_file = NULL, *gasv_file = NULL, *gasvpro_file = NULL;
  unsigned long GENOME_LENGTH = DEFAULT_GENOME_LENGTH;
  if (GASV_PRO && !WRITE_CONCORDANT) {
    if (VERBOSE) printf("-WRITE_CONCORDANT is required for -GASVPRO output. Automatically enabling.\n\n");
    WRITE_CONCORDANT = 1;    
  }
  if (AMBIGUOUS && !QNAME_SORTED) {
    if (VERBOSE) printf("-QNAME_SORTED is required for -AMBIGUOUS output. Automatically enabling.\n\n");
    QNAME_SORTED = 1; 
  }
  if (FILENAME == CUTOFF_MODE && !LIB_SEP) {
    if (VERBOSE) printf("-LIB_SEP sep is required for -CUTOFF_LMINLMAX FILE=... output. Automatically enabling.\n\n");
    LIB_SEP = 1;     
  }
  if (AMBIGUOUS && (WRITE_LOWQ || WRITE_SPLITREAD)) {
    if (VERBOSE) printf("-WRITE_LOWQ and -WRITE_SPLITREAD behavior are undefined for -AMBIGUOUS. Automatically disabling\n\n");
    WRITE_LOWQ = WRITE_SPLITREAD = 0; 
  }

  // Open all external resources and abort if failure.
  if ((ERR = _open_files(&in, &chromosome_naming_file, &cutoff_file, &lowq_file, &split_file, &info_file, &gasv_file, &gasvpro_file, BAM_PATH, OUTPUT_PREFIX, CHROMOSOME_NAMING, CUTOFF_MODE, CUTOFF_NAME, WRITE_LOWQ, WRITE_SPLITREAD, GASV_PRO))) return _cleanup(in, chromosome_naming_file, cutoff_file, lowq_file, split_file, info_file, gasv_file, gasvpro_file, pairtable, ERR);

  // Allocate hash table
  struct stat st;
  stat(BAM_PATH, &st);  
  if (!(pairtable = hash(!QNAME_SORTED * ((st.st_size / B2G_HASH_SIZE) / (LOW_MEMORY ? MAX_BAM_FILES : 1))))) return _cleanup(in, chromosome_naming_file, cutoff_file, lowq_file, split_file, info_file, gasv_file, gasvpro_file, pairtable, B2GERR_OUT_OF_MEMORY);

  // Calculate required space for bam_header metadata,
  int NUM_CHROMOSOMES, NUM_READGROUPS, NUM_LIBRARIES;
  b2g_bam_header_counts(in->header, &NUM_CHROMOSOMES, &NUM_READGROUPS, &NUM_LIBRARIES, &LIB_SEP, PROPER_LENGTH);
    
  // allocate memory for said data,
  b2g_cutoff_lminlmax_t cutoff_lminlmax = {CUTOFF_MODE, CUTOFF_X , CUTOFF_Y, CUTOFF_NAME};
  int chromosome_numbers[NUM_CHROMOSOMES];
  b2g_chromosomes_t chromosomes = {NUM_CHROMOSOMES, chromosome_numbers};
  b2g_readgroup2library_t rgs2libs[NUM_READGROUPS];
  b2g_library_t libs[NUM_LIBRARIES];
  b2g_libraries_t libraries = {NUM_READGROUPS, NUM_LIBRARIES, &rgs2libs[0], &libs[0]};

  // and read the header into that memory.
  if ((ERR = b2g_bam_header_read(in->header, &chromosomes, &libraries, &GENOME_LENGTH, cutoff_lminlmax, chromosome_naming_file, cutoff_file, LIB_SEP, USE_NUMBER_READS, PROPER_LENGTH, VERBOSE))) return _cleanup(in, chromosome_naming_file, cutoff_file, lowq_file, split_file, info_file, gasv_file, gasvpro_file, pairtable, ERR);

  // Do all the main program logic of splitting the bam file, sorting the contents, merging the final output, and dumping the statistics.
  if ((ERR = _split_sort_merge_dump(BAM_PATH, OUTPUT_PREFIX, MAPPING_QUALITY, WRITE_CONCORDANT, WRITE_LOWQ, LOW_MEMORY, AMBIGUOUS, LIB_SEP, VERBOSE, CUTOFF_X, CUTOFF_Y, CUTOFF_MODE, CUTOFF_NAME, PROPER_LENGTH, USE_NUMBER_READS, DEBUG_LEVEL, WRITE_SPLITREAD, MIN_ALIGNED_PCT, CHROMOSOME_NAMING, PLATFORM, GASV_PRO, IGNORE_DUPLICATES, lowq_file, split_file, &libraries, &chromosomes, pairtable, info_file, gasv_file, gasvpro_file, GENOME_LENGTH, QNAME_SORTED, SPLIT_BY_CHROMOSOME, num_skipped))) return _cleanup(in, chromosome_naming_file, cutoff_file, lowq_file, split_file, info_file, gasv_file, gasvpro_file, pairtable, ERR);

  return _cleanup(in, chromosome_naming_file, cutoff_file, lowq_file, split_file, info_file, gasv_file, gasvpro_file, pairtable, B2GERR_NO_ERROR);
}
