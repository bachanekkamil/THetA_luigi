#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "disc_pair.h"
#include "conc_pair.h"
#include "gasv_signal.h"
#include "io.h"



int main() {

  // CONSTRUCTORS

  int chr = 24, lstart = 0, rend = 2147483647;
  conc_pair_t *pair = conc_pair(chr, lstart, rend);
  conc_pair_t *pair2 = conc_pair(1, 10, 100);
  conc_pair_t *pair3 = conc_pair(1, 10, 1000);
  conc_pair_t pair4;

  assert(conc_pairp(pair));
  assert(!gasv_signalp(pair));
  assert(!disc_pairp(pair));
  pair->conc = 0;
  assert(!conc_pairp(pair));
  assert(disc_pairp(pair));
  pair->conc = 1;


  // ACCESSORS

  assert(conc_pair_chr(pair) == chr);
  assert(conc_pair_lstart(pair) == lstart);
  assert(conc_pair_rend(pair) == rend);

  // PREDICATES

  assert(0 <= conc_pair_cmp(pair, pair2));
  assert(0 <= conc_pair_cmp(pair3, pair2));
  assert(0 <= conc_pair_cmp(pair, pair3));
  
  // I/O

    FILE *out = file_open("tmp/conc_pair/conc_pair.gasv", "w");
  conc_pair_write(pair, out);
  fclose(out);

  FILE *in = file_open("tmp/conc_pair/conc_pair.gasv", "r");
  assert(0 < conc_pair_read(&pair4, in));
  fclose(in);

  assert(!conc_pair_cmp(pair, &pair4));

  conc_pair_free(pair);
  conc_pair_free(pair2);
  conc_pair_free(pair3);

  return 0;
}
