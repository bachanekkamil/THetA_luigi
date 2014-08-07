// This function allows a thread to become the BAM input reader thread, reading the entire contents of all input BAM files into a buffer before returning. Other threads must have been started previously to empty that buffer as this will block until all input has been consumed.
// Output consists of a single buffer of b2g_bam_t's separated by B2G_SIGNAL_EOFs to mark the end of each input file.

#ifndef B2G_BAM_FILES2BAMS_H
#define B2G_BAM_FILES2BAMS_H

#include "buffer.h"
#include "b2g_chromosome.h"

void bam_files2bams(char *, char *, int , int, buffer_t *, b2g_chromosomes_t *, int, unsigned int *);

#endif
