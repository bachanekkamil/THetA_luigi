// Represents a generic interface to b2g_disc_pair_t and b2g_conc_pair_t that allows dynamic dispatching so that the same code can handle both types.

#ifndef B2G_PAIR_H
#define B2G_PAIR_H

#include "list.h"
#include "b2g_disc_pair.h"
#include "b2g_chromosome.h"
#include "b2g_conc_pair.h"
#include "b2g_bam.h"
#include "b2g_variant_type.h"
#include "b2g_constants.h"

// UTILITY

// Get the size needed for a single disc/conc pair based on the variant_type
#define gasv_sizeof(pair_type) (CONC == (pair_type) ? sizeof(b2g_conc_pair_t) : sizeof(b2g_disc_pair_t))

// DATATYPES

// A tagged union representing b2g_disc_pair and b2g_conc_pair types. The tag is bit-packed into the b2g_disc_pair_t and b2g_conc_pair_t types.
typedef struct {
  unsigned int __filler; // Used only to position the conc flag.
  unsigned int conc:1; // This flag must be at the same place in disc and conc pairs to facilitate dynamic dispatch.
} b2g_pair_t;

// CONSTRUCTORS

void b2g_pair_free(b2g_pair_t *);
void b2g_pair_clear(b2g_pair_t *);
//b2g_pair_t *bams2pair(b2g_bam_t *, b2g_bam_t *, char *, b2g_chromosomes_t *, unsigned char, int, int, int, b2g_platform_t);

// PREDICATES

#define b2g_pair_convergent(gpair) b2g_disc_pair_convergent((b2g_disc_pair_t *)gpair)
#define b2g_pair_lchr(pair) (b2g_conc_pairp(pair) ? b2g_conc_pair_chr((b2g_conc_pair_t *)pair) : b2g_disc_pair_lchr((b2g_disc_pair_t *)pair))
#define b2g_pair_rchr(pair) (b2g_conc_pairp(pair) ? b2g_conc_pair_chr((b2g_conc_pair_t *)pair) : b2g_disc_pair_rchr((b2g_disc_pair_t *)pair))
#define b2g_pair_equal_chr(gpair) (b2g_conc_pairp((b2g_pair_t *)gpair) || b2g_disc_pair_equal_chr((b2g_disc_pair_t *)(gpair)))
int b2g_pair_cmp(b2g_pair_t *, b2g_pair_t *);

// ACCESSORS

b2g_variant_t b2g_pair_vtype(b2g_pair_t *, int, int, b2g_platform_t);

// AGGREGATES

int b2g_pair_sort(list_t *, list_t *);

// I/O

// Human readable debug print.
void b2g_pair_print(b2g_pair_t *);
// File write with internal representation to be read back in with read.
void b2g_pair_write(b2g_pair_t *, FILE *);
// Write output-formatted final representation. Cannot be read back in with read.
void b2g_pair_display(b2g_pair_t *, FILE *, b2g_chromosomes_t *);
// Read back in file line  formatted with write. Returns bytes read.
int b2g_pair_read(b2g_pair_t *, FILE *, b2g_variant_t );
  
#endif
