// Provides utilities for extracting BAMToGASV-relevant data from samtools' bam_header_t.

#ifndef B2G_BAM_HEADER_H
#define B2G_BAM_HEADER_H

#include <stdio.h>
#include "b2g_chromosome.h"
#include "b2g_error.h"
#include "bam.h"
#include "b2g_libraries.h"

// Parses the header and returns the number of chromosomes, readgroups, and libraries. Used for allocating space before reading aforementioned data with b2g_bam_header_read.
void b2g_bam_header_counts(bam_header_t *, int *, int *, int *, int *, int);

// Parses the header and fills up the chromosome and libraries structures, using data from CHROMOSOME_NAMING and CUTOFF_LMINLMAX as needed.
b2g_error_t b2g_bam_header_read(bam_header_t *, b2g_chromosomes_t *, b2g_libraries_t *, unsigned long *, b2g_cutoff_lminlmax_t, FILE *, FILE *, int, int, int, int);


#endif
