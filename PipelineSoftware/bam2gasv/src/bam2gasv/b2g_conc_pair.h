// Provides a compressed representation of a pair that we know is concordant.

#ifndef B2G_CONC_PAIR_H
#define B2G_CONC_PAIR_H

#include "list.h"
#include "bam.h"
#include "b2g_signal.h"
#include "b2g_bam.h"
#include "b2g_disc_pair.h"
#include "b2g_chromosome.h"

// DATATYPES

typedef struct {
  unsigned int lstart; // Must start with a positive int such as lstart: the gasv protocol escapes bams and pairs with negative ints.
  unsigned int conc:1; // conc flag must be at the same offset in b2g_conc_pair_t and b2g_disc_pair_t, as it is used to type-check them.
  unsigned int chr:31; // Use whatever spare room we have in the struct to store the chr.
  unsigned int rend;  
} b2g_conc_pair_t;

// CONSTRUCTORS

b2g_conc_pair_t *b2g_conc_pair(unsigned int, unsigned int, unsigned int);
void b2g_conc_pair_init(b2g_conc_pair_t *, unsigned int, unsigned int, unsigned int);
// Constructs a conc pair from a disc pair non-destructively.
b2g_conc_pair_t *b2g_disc_pair2b2g_conc_pair(b2g_disc_pair_t *);
//b2g_conc_pair_t *bams2b2g_conc_pair(b2g_bam_t *, b2g_bam_t *, b2g_chromosomes_t *, unsigned char);
void b2g_conc_pair_free(b2g_conc_pair_t *);


// ACCESSORS

#define b2g_conc_pair_chr(pair) ((pair)->chr)
#define b2g_conc_pair_lstart(pair) ((pair)->lstart)
#define b2g_conc_pair_rend(pair) ((pair)->rend)


// PREDICATES

#define b2g_conc_pairp(pair) (!b2g_signalp(pair) && (pair)->conc)
int b2g_conc_pair_cmp(b2g_conc_pair_t*, b2g_conc_pair_t*);


// I/O

// Human readable debug print.
void b2g_conc_pair_print(b2g_conc_pair_t *);
// File write with internal representation to be read back in with read.
void b2g_conc_pair_write(b2g_conc_pair_t *, FILE *);
// Write output-formatted final representation. Cannot be read back in with read.
void b2g_conc_pair_display(b2g_conc_pair_t *, FILE *, b2g_chromosomes_t *);
// Read back in file line  formatted with write. Returns bytes read.
int b2g_conc_pair_read(b2g_conc_pair_t *, FILE *);

#endif
