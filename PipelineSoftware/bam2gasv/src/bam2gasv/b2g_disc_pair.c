#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "b2g_disc_pair.h"
#include "b2g_signal.h"
#include "b2g_variant_type.h"
#include "b2g_bam.h"
#include "b2g_chromosome.h"
#include "b2g_constants.h"
#include "str.h"



// CONSTRUCTORS

b2g_disc_pair_t *b2g_disc_pair(char* qname, unsigned int lchr, unsigned int lstart, unsigned int lend, unsigned int lori, unsigned int rchr, unsigned int rstart, unsigned int rend, unsigned int rori) {
  b2g_disc_pair_t *pair = b2g_malloc(sizeof(b2g_disc_pair_t));
  b2g_disc_pair_init(pair, qname, lchr, lstart, lend, lori, rchr, rstart, rend, rori);
  return pair;
}

void b2g_disc_pair_init(b2g_disc_pair_t *pair, char* qname, unsigned int lchr, unsigned int lstart, unsigned int lend, unsigned int lori, unsigned int rchr, unsigned int rstart, unsigned int rend, unsigned int rori) {
  assert(qname && is_bool(lori) && is_bool(rori));

  pair->qname = qname;
  pair->lstart = lstart;
  pair->rstart = rstart;
  pair->lori = lori;
  pair->rori = rori;
  pair->rchr = rchr;
  pair->lchr = lchr;
  pair->lend = lend;
  pair->rend = rend;
  pair->conc = 0;
}

static b2g_disc_pair_t *_make_disc_pair(char* qname, unsigned int lchr, unsigned int lstart, unsigned int lend, unsigned int lori, unsigned int rchr, unsigned int rstart, unsigned int rend, unsigned int rori, b2g_chromosomes_t *chromosomes) {
  if (-1 == lchr || -1 == rchr) return NULL;
  return b2g_disc_pair(qname, lchr, lstart, lend, lori, rchr, rstart, rend, rori);
}

b2g_disc_pair_t *b2g_bam2disc_pair(b2g_bam_t *b1, char *qname, b2g_chromosomes_t *chromosomes, b2g_platform_t PLATFORM) {
  assert(b1 && b2g_bam_qname(b1));
  
  // Ensure the left bam is b1.
  int swap_reads = (0 < b2g_bam_mate_cmp(b1, chromosomes));
  int first_is_first = swap_reads ? !b2g_bam_first_of_pair(b1) : b2g_bam_first_of_pair(b1);  

  return _make_disc_pair(str_copy(qname),
			 swap_reads ? b2g_bam_mate_chr(b1, chromosomes) : b2g_bam_chr(b1, chromosomes),
			 swap_reads ? b2g_bam_mate_start(b1) : b2g_bam_start(b1),
			 swap_reads ? b2g_bam_mate_end(b1) : b2g_bam_end(b1),
			 swap_reads ? b2g_bam_lori_normalized(b2g_bam_mate_ori(b1), PLATFORM) : b2g_bam_lori_normalized(b2g_bam_ori(b1), PLATFORM),
			 swap_reads ? b2g_bam_chr(b1, chromosomes) : b2g_bam_mate_chr(b1, chromosomes),
			 swap_reads ? b2g_bam_start(b1) : b2g_bam_mate_start(b1),
			 swap_reads ? b2g_bam_end(b1) : b2g_bam_mate_end(b1),
			 swap_reads ? b2g_bam_rori_normalized(b2g_bam_ori(b1), PLATFORM, first_is_first) : b2g_bam_rori_normalized(b2g_bam_mate_ori(b1), PLATFORM, first_is_first),
			 chromosomes);
}

b2g_disc_pair_t *b2g_bams2disc_pair(b2g_bam_t *b1, b2g_bam_t *b2, char *qname, b2g_chromosomes_t *chromosomes, b2g_platform_t PLATFORM) {
  assert(b1 && b2g_bam_qname(b1) && b2);
  
  // Ensure the left bam is b1.
  b2g_bam_t *swap = b1;
  if (0 < b2g_bam_mate_cmp(b1, chromosomes)) {
    b1 = b2;
    b2 = swap;
  }

  int first_is_first = b2g_bam_first_of_pair(b1);  
  return _make_disc_pair(str_copy(qname),
			 b2g_bam_chr(b1, chromosomes),
			 b2g_bam_start(b1),
			 b2g_bam_end(b1),
			 b2g_bam_lori_normalized(b2g_bam_ori(b1), PLATFORM),
			 b2g_bam_chr(b2, chromosomes),
			 b2g_bam_start(b2),
			 b2g_bam_end(b2),
			 b2g_bam_rori_normalized(b2g_bam_ori(b2), PLATFORM, first_is_first),
			 chromosomes);
}

void b2g_disc_pair_free(b2g_disc_pair_t *pair) {
  assert(pair);
  assert(pair->qname);
  assert(!b2g_signalp(pair));
  assert(b2g_disc_pairp(pair));
  free(pair->qname);
  free(pair);
}

// Free only the qname, leaving the pair's memory in tact.
void b2g_disc_pair_clear(b2g_disc_pair_t *pair) {
  assert(pair);
  assert(pair->qname);
  assert(!b2g_signalp(pair));
  assert(b2g_disc_pairp(pair));
  
  free(pair->qname); 
  bzero(pair, sizeof(b2g_disc_pair_t));
}


// PREDICATES

int b2g_disc_pair_cmp(b2g_disc_pair_t* a, b2g_disc_pair_t* b) {
  assert(a && b);
  assert(!b2g_signalp(a));
  assert(!b2g_signalp(b));
  assert(b2g_disc_pairp(a));
  assert(b2g_disc_pairp(b));


  int left_bams_cmp =  b2g_cmp(b2g_disc_pair_lchr(a), b2g_disc_pair_lchr(b),
		 b2g_disc_pair_lstart(a), b2g_disc_pair_lstart(b),
		 b2g_disc_pair_lend(a), b2g_disc_pair_lend(b));

  // If the left bams are equal,
  if (!left_bams_cmp) {
    // compare the right bams.
    return b2g_cmp(b2g_disc_pair_rchr(a), b2g_disc_pair_rchr(b),
		   b2g_disc_pair_rstart(a), b2g_disc_pair_rstart(b),
		   b2g_disc_pair_rend(a), b2g_disc_pair_rend(b));
  }
  // Otherwise, the left bams dominate the comparison.
  else return left_bams_cmp;
}

// ACCESSORS

b2g_variant_t b2g_disc_pair_vtype(b2g_disc_pair_t *pair, int lmin, int lmax) {
  assert(pair && b2g_disc_pairp(pair));
  return variant_type(b2g_disc_pair_lchr(pair), 
		      b2g_disc_pair_rchr(pair), 
		      b2g_disc_pair_lori(pair), 
		      b2g_disc_pair_rori(pair), 
		      b2g_disc_pair_lstart(pair),
		      b2g_disc_pair_rend(pair), 
		      lmin, lmax);
}

// I/O

void b2g_disc_pair_print(b2g_disc_pair_t *pair) {
  assert(pair);
  if (b2g_signal_eof(pair)) printf("#B2G_SIGNAL_EOF");
  else if (b2g_signal_err(pair)) printf("#B2G_SIGNAL_ERR");
  else {
    assert(b2g_disc_pairp(pair));
    printf("#b2g_disc_pair_t(%s %d %d %d %d %d %d %d %d)", b2g_disc_pair_qname(pair), b2g_disc_pair_lchr(pair), b2g_disc_pair_lstart(pair), b2g_disc_pair_lend(pair), b2g_disc_pair_lori(pair), b2g_disc_pair_rchr(pair), b2g_disc_pair_rstart(pair), b2g_disc_pair_rend(pair), b2g_disc_pair_rori(pair));
  }
}

void b2g_disc_pair_write(b2g_disc_pair_t *pair, FILE *out) {
  assert(pair && pair->qname);
  assert(b2g_disc_pairp(pair));
  fprintf(out, "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\n", b2g_disc_pair_qname(pair), b2g_disc_pair_lchr(pair), b2g_disc_pair_lstart(pair), b2g_disc_pair_lend(pair), b2g_disc_pair_lori(pair), b2g_disc_pair_rchr(pair), b2g_disc_pair_rstart(pair), b2g_disc_pair_rend(pair), b2g_disc_pair_rori(pair));
}

void b2g_disc_pair_display(b2g_disc_pair_t *pair, FILE *out, b2g_chromosomes_t *chromosomes) {
  assert(pair && pair->qname && b2g_disc_pairp(pair) && is_nonnegative(b2g_disc_pair_lchr(pair)) && is_nonnegative(b2g_disc_pair_rchr(pair)));
  fprintf(out, "%s\t%d\t%d\t%d\t%c\t%d\t%d\t%d\t%c\n", b2g_disc_pair_qname(pair), b2g_disc_pair_lchr(pair), b2g_disc_pair_lstart(pair), b2g_disc_pair_lend(pair), ORIENTATION_CHARS[b2g_disc_pair_lori(pair)], b2g_disc_pair_rchr(pair), b2g_disc_pair_rstart(pair), b2g_disc_pair_rend(pair), ORIENTATION_CHARS[b2g_disc_pair_rori(pair)]);
}

int b2g_disc_pair_read(b2g_disc_pair_t *pair, FILE *in) {
  assert(in);
  if (!pair) return 0;

  int lchr, rchr, lori, rori, amt_read;
  char qname[256];
  if (-1 == fscanf(in, "%s %d %d %d %d %d %d %d %d\n%n", qname, &lchr, &pair->lstart, &pair->lend, &lori, &rchr, &pair->rstart, &pair->rend, &rori, &amt_read)) {    
    bzero(pair, sizeof(b2g_disc_pair_t));
    return B2G_SIGNAL_EOF;
  }
  
  pair->qname = b2g_malloc(sizeof(char) * (strlen(qname) + 1));
  strcpy(pair->qname, qname);
  pair->lori = lori;
  pair->rori = rori;
  pair->lchr = lchr;
  pair->rchr = rchr;
  pair->conc = 0;
  assert(b2g_disc_pairp(pair));
  return amt_read;
}
