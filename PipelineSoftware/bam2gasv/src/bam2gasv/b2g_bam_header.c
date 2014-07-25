#include <assert.h>
#include <stdio.h>
#include "bam.h"
#include "b2g_bam_header.h"
#include "b2g_error.h"
#include "b2g_chromosome.h"
#include "khash.h"
#include "b2g_libraries.h"

// DATATYPES

KHASH_MAP_INIT_STR(str, const char *) // Define a khash_t(str) type hashtable. This is imported from samtools for reading the header structure.


// HEADER PARSING

// Add the {readgroup_name, library} to rgs2libs and increment the readgroups_count.
void _add_readgroup(b2g_readgroup2library_t rgs2libs[], int library_index, int readgroups_count, const char *readgroup_name) {
  strcpy(rgs2libs[readgroups_count].name, readgroup_name);
  rgs2libs[readgroups_count].library_index = library_index;  
}

// Extract readgorups, libraries and their respective counts from the header.
void _header2readgroups_libraries(bam_header_t *header, int *NUM_READGROUPS, int *NUM_LIBRARIES, b2g_readgroup2library_t rgs2libs[], b2g_library_t libraries[], int *LIB_SEP, b2g_cutoff_lminlmax_t *cutoff_parameters, int PROPER_LENGTH) {
  assert(header);
  int i, readgroups_count = 0, libraries_count = 0;
  khint_t k;  
  khash_t(str) *h = (khash_t(str)*)header->rg2lib;
  
  // Either loop through the rgs and libs and store them in provided memory,
  for (k = kh_begin(h); k<kh_end(h) && *LIB_SEP; k++) {
    if (kh_exist(h, k)) { // If the hash buckeqt exists,
      for (i=0; i<libraries_count; i++) { // check if the library has been seen.
	// If we have seen this library before, skip it.
	if (!strcmp(libraries[i].name, kh_val(h, k))) goto library_seen;
      }
      // Otherwise, count it and add it to the already-seen list.
     b2g_library_init(&libraries[libraries_count++], kh_val(h, k), cutoff_parameters, PROPER_LENGTH);
    library_seen: _add_readgroup(rgs2libs, i, readgroups_count++, kh_key(h, k)); // In either case add the readgroup + pointer 
    }
  }
  // or set the default "all" lib.
  if (!libraries_count) {
    b2g_library_init(&libraries[libraries_count++], "all", cutoff_parameters, PROPER_LENGTH);
    *LIB_SEP = 0;
  }
  assert(0 < libraries_count);
  if (NUM_READGROUPS) *NUM_READGROUPS = readgroups_count;
  if (NUM_LIBRARIES) *NUM_LIBRARIES = libraries_count;
}


// Return counts of how many chromosomes, readgroups, and libraries this header contains for use in allocating space for them.
void b2g_bam_header_counts(bam_header_t *header, int *NUM_CHROMOSOMES, int *NUM_READGROUPS, int *NUM_LIBRARIES, int *LIB_SEP, int PROPER_LENGTH) {
  assert(header && NUM_CHROMOSOMES && NUM_READGROUPS && NUM_LIBRARIES);
  *NUM_CHROMOSOMES = header->n_targets;    
  int readgroups_count, libraries_count;
  khash_t(str) *h = (khash_t(str)*)header->rg2lib;

  // Allocate dummy space since we only need counts here. 
  b2g_library_t libraries[kh_end(h) ? kh_end(h) : 1]; // kh_end(h) is an upper bound and 1 is a lower bound (in case of "all").
  b2g_readgroup2library_t readgroups[kh_end(h)]; // No need for a lower bound since this may be empty in the "all" case.
  b2g_cutoff_lminlmax_t cutoff_lminlmax;

  _header2readgroups_libraries(header, &readgroups_count, &libraries_count, readgroups, libraries, LIB_SEP, &cutoff_lminlmax, PROPER_LENGTH);  

  assert(0 < libraries_count && 0 < *NUM_CHROMOSOMES);
  *NUM_READGROUPS = readgroups_count;
  *NUM_LIBRARIES = libraries_count;
}

b2g_error_t b2g_bam_header_read(bam_header_t *header, b2g_chromosomes_t *chromosomes, b2g_libraries_t *libraries, unsigned long *GENOME_LENGTH, b2g_cutoff_lminlmax_t cutoff_lminlmax, FILE *chromosome_naming_file, FILE *cutoff_file, int LIB_SEP, int USE_NUMBER_READS, int PROPER_LENGTH, int VERBOSE) {
  assert(header && chromosomes && libraries);

  int i, NUM_LIBS, some_chrs_skipped = 0;
  b2g_error_t ERR = B2GERR_NO_ERROR;

  // Extract the basic data from the header,
  _header2readgroups_libraries(header, NULL, &NUM_LIBS, libraries->readgroups2libraries, libraries->libraries, &LIB_SEP, &cutoff_lminlmax, PROPER_LENGTH);
  if (header->target_len && *header->target_len) *GENOME_LENGTH = *header->target_len;

  // then attempt to post-process the data with additional user input.
  if ((ERR = b2g_chromosomes_read(chromosome_naming_file, header, chromosomes, VERBOSE))) return ERR;
  for (i=0; VERBOSE && i<chromosomes->length; i++) {
    if (-1 == b2g_chromosomes_numbers_get(chromosomes, i)) {
      printf("%s%s", some_chrs_skipped++ ? ", " : "Skipping unrecognized chromosome names: ", header->target_name[i]);
    }
  }
  if (some_chrs_skipped) printf("\n\n");

  if ((ERR = b2g_library_cutoff_lminlmax_read(cutoff_file, libraries->libraries, libraries->libraries_length, VERBOSE))) return ERR;

  return ERR;
}
