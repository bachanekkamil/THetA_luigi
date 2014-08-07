// Provides a datatype representing a discordant pair (or a pair that has not yet been determined to be concordant if lmin and lmax still need to be calculated).

#ifndef B2G_DISC_PAIR_H
#define B2G_DISC_PAIR_H

#include <stdio.h>
#include "b2g_chromosome.h"
#include "b2g_constants.h"
#include "b2g_variant_type.h"
#include "b2g_signal.h"
#include "b2g_bam.h"
#include "list.h"

// DATATYPES

typedef struct {
  unsigned int lstart; // Must start with a positive int such as lstart: the gasv protocol escapes bams and pairs with negative ints.
  unsigned int conc:1; // conc flag must be at the same offset in b2g_conc_pair_t and b2g_disc_pair_t, as it is used to type-check them.  
  unsigned int lchr:14; // Split any extra room in the struct alignment between the chrs.
  unsigned int rchr:15;
  unsigned int rori:1;
  unsigned int lori:1;  
  char* qname;  
  unsigned int lend, rstart, rend;    
} b2g_disc_pair_t;

// CONSTRUCTORS

b2g_disc_pair_t* b2g_disc_pair(char*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
void b2g_disc_pair_init(b2g_disc_pair_t *, char*, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int);
b2g_disc_pair_t *b2g_bam2disc_pair(b2g_bam_t *, char *, b2g_chromosomes_t *, b2g_platform_t);
b2g_disc_pair_t *b2g_bams2disc_pair(b2g_bam_t *, b2g_bam_t *, char *, b2g_chromosomes_t *, b2g_platform_t);
void b2g_disc_pair_free(b2g_disc_pair_t *);
void b2g_disc_pair_clear(b2g_disc_pair_t *);


// PREDICATES

#define b2g_disc_pairp(pair) (!b2g_signalp(pair) && !(pair)->conc)
#define b2g_disc_pair_convergent(dpair) (0 == (dpair)->lori && 1 == (dpair)->rori)
#define b2g_disc_pair_equal_chr(dpair) ((dpair)->lchr == (dpair)->rchr)
int b2g_disc_pair_cmp(b2g_disc_pair_t *a, b2g_disc_pair_t *);


// ACCESSORS

#define b2g_disc_pair_qname(pair) ((pair)->qname)
#define b2g_disc_pair_lchr(pair) ((pair)->lchr)
#define b2g_disc_pair_rchr(pair) ((pair)->rchr)
#define b2g_disc_pair_lori(pair) ((pair)->lori)
#define b2g_disc_pair_rori(pair) ((pair)->rori)
#define b2g_disc_pair_lstart(pair) ((pair)->lstart)
#define b2g_disc_pair_rstart(pair) ((pair)->rstart)
#define b2g_disc_pair_lend(pair) ((pair)->lend)
#define b2g_disc_pair_rend(pair) ((pair)->rend)
#define b2g_disc_pair_readlength(pair) ((pair)->lend - (pair)->lstart + 1)
#define b2g_disc_pair_insert_len(dpair) (b2g_disc_pair_rend(dpair) - b2g_disc_pair_lstart(dpair) + 1)
b2g_variant_t b2g_disc_pair_vtype(b2g_disc_pair_t *, int, int);


// I/O

// Human readable debug output.
void b2g_disc_pair_print(b2g_disc_pair_t *);
// File write with internal representation to be read back in with read.
void b2g_disc_pair_write(b2g_disc_pair_t *, FILE *);
// Write output-formatted final representation. Cannot be read back in with read.
void b2g_disc_pair_display(b2g_disc_pair_t *, FILE *, b2g_chromosomes_t *);
// Read back in file line  formatted with write. Returns bytes read.
int b2g_disc_pair_read(b2g_disc_pair_t *, FILE *);

#endif 
