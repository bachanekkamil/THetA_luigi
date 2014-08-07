#ifndef B2G_COMPRESSED_BAM_H
#define B2G_COMPRESSED_BAM_H

#include "b2g_bam.h"

// DATATYPES

typedef struct {
  char highq;
  char qname[];
} b2g_compressed_bam_t;

// Used for packaging extra information that b2g_bam2compressed_bam needs to compress the bam.
typedef struct {
  int MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES;
  b2g_bam_t *bam;
} b2g_extended_bam_t;

// CONSTRUCTORS

b2g_compressed_bam_t *b2g_extended_bam2compressed_bam(b2g_extended_bam_t *);
#define b2g_compressed_bam_free free

// PREDICATES

#define b2g_compression_on(QNAME_SORTED, WRITE_LOWQ) (!QNAME_SORTED && !WRITE_LOWQ)
#define b2g_compressed_bam_highq(bam) ((bam)->highq)
int b2g_compressed_bam_qname_cmp(void* , void* );
int b2g_bam_compressed_bam_highq(b2g_bam_t *, b2g_compressed_bam_t *, int, int, int);

// ACCESSORS

#define b2g_compressed_bam_qname(bam) ((bam)->qname)
#define b2g_extended_bam_qname(b) (b2g_bam_qname((b)->bam))

#endif
