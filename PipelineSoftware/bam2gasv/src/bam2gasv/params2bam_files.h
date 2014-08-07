//Reads once through the bam file and splits it up into many smaller temporary bam files (each less than the size of main memory). Two reads will end up in the same temporary file if their qnames hash to the same value. Later, each file will be read in entirety into memory, and we can then pair up like-qname reads in memory to generate the paired outputs (since all reads with the same qname should have ended up in the same file).

#ifndef B2G_PARAMS2BAM_FILES_H
#define B2G_PARAMS2BAM_FILES_H

#include "b2g_chromosome.h"

int params2bam_files(char *, char *, int, int, int, int, b2g_chromosomes_t *);

#endif
