// Provides a datatype representing a collection of b2g_library_t for use in directing pairs to the appropriate b2g_library_t for processing, and in printing aggregated statistics over all libraries for input to GASVPRO.

#ifndef B2G_LIBRARIES_H
#define B2G_LIBRARIES_H

#include <stdio.h>
#include "b2g_cutoff_lminlmax.h"
#include "b2g_bam.h"
#include "b2g_pair.h"
#include "b2g_constants.h"
#include "b2g_library.h"
#include "buffer.h"
#include "b2g_chromosome.h"
#include "b2g_compressed_bam.h"

// DATATYPES

// A single cell in a lookup table for mapping readgroup names to library indices in the b2g_libraries_t.
typedef struct {
  char name[MAX_READGROUP_LENGTH];
  int library_index;
} b2g_readgroup2library_t;

// A collection of libraries.
typedef struct {
  int readgroups2libraries_length, libraries_length; // number of readgroups or libraries contained in BAM file.
  b2g_readgroup2library_t *readgroups2libraries; // A lookup table for mapping readgroup names to library indices in the b2g_libraries_t.
  b2g_library_t *libraries; // The list of libraries.
} b2g_libraries_t;


// ACCESSORS

#define b2g_libraries_length(libs) ((libs)->libraries_length)
#define b2g_libraries_get(libs, i) (&(libs)->libraries[i])


// Signals that a library currently accumulating pairs for use in computing statistics should simply compute with the pairs it has and write all its pairs because there will be no more input forthcoming.
void b2g_libraries_flush(b2g_libraries_t *, list_t *, int, int, b2g_platform_t, int);

// Writes a single pair to the appropriate buffer by matching it to the appropriate library and computing its variant_type based on that library's statistics.
void b2g_libraries_write(b2g_libraries_t *, char *, b2g_bam_t *, b2g_bam_t *, list_t *, buffer_t *, b2g_chromosomes_t *, int, int, int, int, int, int, b2g_platform_t, int, int, int, int);
void b2g_libraries_write_ambiguous(b2g_libraries_t *, b2g_bam_t *, list_t *, list_t *, buffer_t *, b2g_chromosomes_t *, int, int, int, int, int, b2g_platform_t, int, int, int);
void b2g_libraries_write_compressed(b2g_libraries_t *, char *, b2g_bam_t *, b2g_compressed_bam_t *, list_t *, b2g_chromosomes_t *, int, int, int, int, int, b2g_platform_t, int, int, int);

void b2g_libraries_write_gasv_in(b2g_libraries_t *, FILE *, char *);
void b2g_libraries_write_info(b2g_libraries_t *, FILE *, int, int);
void b2g_libraries_write_gasvpro_in(b2g_libraries_t *, FILE *, char *, int, unsigned long);



#endif
