#include <string.h>
#include "b2g_compressed_bam.h"
#include "b2g_bam.h"

// CONSTRUCTORS

b2g_compressed_bam_t *b2g_extended_bam2compressed_bam(b2g_extended_bam_t *extended_bam) {
  b2g_bam_t *bam = extended_bam->bam;
  b2g_compressed_bam_t *compressed_bam = b2g_malloc(sizeof(b2g_compressed_bam_t) + b2g_bam_qname_length(bam));
  compressed_bam->highq = HIGH == b2g_bam_type(bam, extended_bam->MAPPING_QUALITY, extended_bam->MIN_ALIGNED_PCT, extended_bam->IGNORE_DUPLICATES);
  strcpy(compressed_bam->qname, b2g_bam_qname(bam));
  return compressed_bam;
}

// PREDICATES

int b2g_compressed_bam_qname_cmp(void* a, void* b) {
  return strcmp(b2g_compressed_bam_qname(((b2g_compressed_bam_t*)a)), b2g_extended_bam_qname((b2g_extended_bam_t*)b));
}

int b2g_bam_compressed_bam_highq(b2g_bam_t *bam1, b2g_compressed_bam_t *bam2, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  return !pos_overlap(b2g_bam_start(bam1), b2g_bam_end(bam1), b2g_bam_mate_start(bam1), b2g_bam_mate_end(bam1)) && 
    HIGH == b2g_bam_type(bam1, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES) && 
    b2g_compressed_bam_highq(bam2);
}
