#include <assert.h>
#include "b2g_variant_type.h"
#include "b2g_constants.h"
#include "b2g_bam.h"

// CONSTANTS

char* VARIANT_EXTENSIONS[] = {"translocation", "inversion", "deletion", "insertion", "divergent", "concordant"};
int VARIANT_TYPES[] = {TRANS, INV, DEL, INS, DIV, CONC};

// CONSTRUCTORS

b2g_variant_t variant_type(int lchr, int rchr, int lori, int rori, int lstart, int rend, int lmin, int lmax) {
  if (lchr != rchr) return TRANS;
  if (lori == rori) return INV;
  if (1 == lori || 0 == rori) return DIV;
  int insert_size = rend - lstart + 1;
  if (lmin <= insert_size && insert_size <= lmax) return CONC;
  if (lmax < insert_size) return DEL;
  return INS;
}


