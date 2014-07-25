// Provides a datatype representing the chromosome names contained in the BAM file.

#ifndef GASV_CHROMOSOME_H
#define GASV_CHROMOSOME_H

#include <stdio.h>
#include "list.h"
#include "bam.h"
#include "b2g_error.h"
#include "b2g_constants.h"

// DATATYPES

typedef struct {
  int length;  
  int *numbers;
} b2g_chromosomes_t;


// ACCESSORS

int b2g_chromosomes_length(b2g_chromosomes_t *);

// Returns the chr sequence number of the b2g_bam_t chromosome id. 
// Returns -1 if the given chr is being skipped, or -2 if the read has no chr.
int b2g_chromosomes_numbers_get(b2g_chromosomes_t *, int);


// SERIALIZATION

// Parses a -CHROMOSOME_NAMING file into an appropriately sized b2g_chromosomes_t structure, or parses the header directly if no CHROMOSOME_NAMING file is supplied (NULL)..
b2g_error_t b2g_chromosomes_read(FILE *, bam_header_t *, b2g_chromosomes_t *, int);

#endif
