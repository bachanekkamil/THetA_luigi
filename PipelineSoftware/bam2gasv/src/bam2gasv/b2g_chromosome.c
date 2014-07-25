#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include "b2g_chromosome.h"
#include "list.h"
#include "bam.h"
#include "b2g_bam.h"
#include "str.h"
#include "sam.h"
#include "sam_header.h"
#include "b2g_constants.h"
#include "str.h"
#include "integer.h"
#include "b2g_error.h"


// ACCESSORS

int b2g_chromosomes_length(b2g_chromosomes_t *chromosomes) {
  assert(chromosomes);
  return chromosomes->length;
}

// Returns the number of the chromosome at index.
int b2g_chromosomes_numbers_get(b2g_chromosomes_t *chromosomes, int index) {
  assert(chromosomes && in_range(index, -1, chromosomes->length - 1));
  if (-1 == index) return -2;  
  return chromosomes->numbers[index];
}



// SERIALIZATION

// Parses something of the form "chr17" into 17. Returns -1 if there was a parse error.
int b2g_chromosome_read(char* chr) {
  int num = -1;
  char suffix[MAX_CHRNAME_LENGTH];
  if (!strncasecmp("chr", chr, 3)) chr = &chr[3];  
  
  if (!strcmp(chr, "X")) return 23;
  else if (!strcmp(chr, "Y")) return 24;

  if (2 == sscanf(chr, "%d%s", &num, suffix)) return -1; // Throw out anything with a suffix after the digit
  return num;
}

b2g_error_t b2g_chromosome_naming_read(FILE *in, bam_header_t *header, b2g_chromosomes_t *chromosomes) {
  assert(in && chromosomes && header);

  int i;
  char name[MAX_CHRNAME_LENGTH];
  int id;
  int num_read;
  for (i=0; i<chromosomes->length; i++) chromosomes->numbers[i] = -1;

  while (-1 != (num_read = fscanf(in, "%s\t%d\n", name, &id))) {
    if (0 == num_read) continue; // Ignore blank lines.
    if (1 == num_read) return B2GERR_CHROMOSOME_NAMING_INVALID_FORMAT; // id must be an integer.

    // Check for any matching chromosome names and overwrite the default -1.
    for (i=0; i<b2g_chromosomes_length(chromosomes); i++) {
      if (!strcmp(header->target_name[i], name)) {
	chromosomes->numbers[i] = id;
      }      
    }
  }

  return B2GERR_NO_ERROR;
}

// Extract chromosome names from the header.
static void _header2chromosomes(bam_header_t *header, b2g_chromosomes_t *chromosomes, int VERBOSE) {
  int i;
  for (i=0; i<header->n_targets; i++) {
    chromosomes->numbers[i] = b2g_chromosome_read(header->target_name[i]);
  }
}

b2g_error_t b2g_chromosomes_read(FILE *in, bam_header_t *header, b2g_chromosomes_t *chromosomes, int VERBOSE) {
  if (!in) _header2chromosomes(header, chromosomes, VERBOSE);
  else return b2g_chromosome_naming_read(in, header, chromosomes);
  return B2GERR_NO_ERROR;
}
