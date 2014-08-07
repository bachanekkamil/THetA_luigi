#include <assert.h>
#include "b2g_pair.h"
#include "b2g_disc_pair.h"
#include "b2g_conc_pair.h"
#include "list.h"
#include "b2g_bam.h"
#include "b2g_variant_type.h"
#include "b2g_chromosome.h"

// CONSTRUCTORS

//TODO remove this
/*
b2g_pair_t *bams2pair(b2g_bam_t *b1, b2g_bam_t *b2, char *qname, b2g_chromosomes_t *chromosomes, unsigned char WRITE_LOWQ, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM) {
  assert(b1 && b2 && b2g_bam_qname(b1)); 
  
  // Ensure the left bam is b1.
  b2g_bam_t *swap;
  if (0 < b2g_bam_cmp(b1, b2, chromosomes, WRITE_LOWQ)) {
    swap = b1;
    b1 = b2;
    b2 = swap;
  }

  // If the reads are not high quality, or
  if (!b2g_bams_highq(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) return NULL;

  // are overlapping, retprn NULL.
  if (b2g_bams_overlap(b1, b2)) {
    //return NULL;
    //TODO
  }

  return (b2g_pair_t *)b2g_bams2b2g_disc_pair(b1, b2, qname, chromosomes, WRITE_LOWQ, PLATFORM);
}
*/

void b2g_pair_free(b2g_pair_t *pair) {
  assert(pair);
  if (b2g_signalp(pair)) return;
  else if (b2g_conc_pairp(pair)) {
    b2g_conc_pair_free((b2g_conc_pair_t *)pair);
  }
  else {
    assert(b2g_disc_pairp(pair));
    b2g_disc_pair_free((b2g_disc_pair_t *)pair);
  }
}

void b2g_pair_clear(b2g_pair_t *pair) {
  assert(pair);
  if (b2g_signalp(pair)) bzero(pair, sizeof(b2g_signal_t));
  else if (b2g_conc_pairp(pair)) {
    bzero(pair, sizeof(b2g_conc_pair_t));
  }
  else if (b2g_disc_pairp(pair)) {
    free(((b2g_disc_pair_t *)pair)->qname);
    bzero(pair, sizeof(b2g_disc_pair_t));
  }
}


// PREDICATES

int b2g_pair_cmp(b2g_pair_t *a, b2g_pair_t *b) {
  if (b2g_conc_pairp(a)) {
    assert(b2g_conc_pairp(b));
    return b2g_conc_pair_cmp((b2g_conc_pair_t *)a, (b2g_conc_pair_t *)b);
  }
  else {
    assert(b2g_disc_pairp(b));
    return b2g_disc_pair_cmp((b2g_disc_pair_t *)a, (b2g_disc_pair_t *)b);
  }
}

// ACCESSORS


b2g_variant_t b2g_pair_vtype(b2g_pair_t *p, int lmin, int lmax, b2g_platform_t PLATFORM) {
  if (b2g_conc_pairp(p)) return CONC;  
  else return b2g_disc_pair_vtype((b2g_disc_pair_t *)p, lmin, lmax);
  // TODO remove these
  //b2g_bam_lori_normalized(b2g_disc_pair_lori(d), PLATFORM), 
  //b2g_bam_rori_normalized(b2g_disc_pair_rori(d), PLATFORM),
}

// AGGREGATES

int b2g_pair_sort(list_t *unsorted, list_t *sorted) {
  if (!unsorted) return 0; // pairss2pair_files may receive a single B2G_SIGNAL_EOF, in which case the cdr would be NULL.
  assert(unsorted->car);  
  assert(sorted);
  
  list_merge_into(sorted, list_mergesort(unsorted, (int (*)(void*, void*))b2g_pair_cmp), (int (*)(void*, void*))b2g_pair_cmp);
  return 1;
}

// I/O

void b2g_pair_print(b2g_pair_t *pair) {
  assert(pair);
  if (b2g_signal_eof(pair)) printf("#B2G_SIGNAL_EOF");
  else if (b2g_signal_err(pair)) printf("#B2G_SIGNAL_ERR");
  else if (b2g_conc_pairp(pair)) b2g_conc_pair_print((b2g_conc_pair_t *)pair);
  else b2g_disc_pair_print((b2g_disc_pair_t *)pair);
}

void b2g_pair_write(b2g_pair_t *pair, FILE *out) {
  assert(pair && out);
  if (b2g_conc_pairp(pair)) {
    b2g_conc_pair_write((b2g_conc_pair_t *)pair, out);
  }
  else {
    assert(b2g_disc_pairp(pair));
    b2g_disc_pair_write((b2g_disc_pair_t *)pair, out);
  }
}

void b2g_pair_display(b2g_pair_t *pair, FILE *out, b2g_chromosomes_t *chromosomes) {
  assert(pair && out && chromosomes);
  if (b2g_conc_pairp(pair)) {
    b2g_conc_pair_display((b2g_conc_pair_t *)pair, out, chromosomes);
  }
  else {
    assert(b2g_disc_pairp(pair));
    b2g_disc_pair_display((b2g_disc_pair_t *)pair, out, chromosomes);
  }
}

int b2g_pair_read(b2g_pair_t *pair, FILE *in, b2g_variant_t vtype) {
  if (!pair) return 0;
  assert(in);
  if (CONC == vtype) {
    return b2g_conc_pair_read((b2g_conc_pair_t *)pair, in);
  }
  else {
    return b2g_disc_pair_read((b2g_disc_pair_t *)pair, in);
  }
}
