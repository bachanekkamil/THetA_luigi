// Datatype representing the variant type of a pair as calculated with respect to its library's statistics.

#ifndef B2G_VARIANT_TYPE_H
#define B2G_VARIANT_TYPE_H


typedef enum {
  TRANS, INV, DEL, INS, DIV, CONC, NUM_VTYPES
} b2g_variant_t;

extern int VARIANT_TYPES[];
extern char* VARIANT_EXTENSIONS[];

// CONSTRUCTORS

b2g_variant_t variant_type(int, int, int, int, int, int, int, int);


#endif
