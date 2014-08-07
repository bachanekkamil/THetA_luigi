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

  char *pair_name = malloc(11 * sizeof(char));
  strcpy(pair_name, "pair_qname");
  char *pair_name2 = malloc(12 * sizeof(char));
  strcpy(pair_name2, "pair_qname2");
  char *pair_name3 = malloc(12 * sizeof(char));
  strcpy(pair_name3, "pair_qname3");

  int lchr = 0, lstart = 1, lend = 10, lori = 0, rchr = 24, rstart = 14, rend = 22, rori = 1;
  
  disc_pair_t *pair = disc_pair(pair_name, 0, lchr, lstart, lend, lori, rchr, rstart, rend, rori);
  disc_pair_t *pair2 = disc_pair(pair_name2, 0, lchr + 1, lstart, lend, lori, rchr, rstart, rend, rori);
  disc_pair_t *pair3 = disc_pair(pair_name3, 0, lchr, lstart, lend, lori, rchr, rstart, rend + 1, rori);
  disc_pair_t pair4;

  // ACCESSORS

  assert(!strcmp(disc_pair_qname(pair), "pair_qname"));
  assert(-1 == disc_pair_lib(pair, 0));
  assert(disc_pair_lchr(pair) == lchr);
  assert(disc_pair_lstart(pair) == lstart);
  assert(disc_pair_lend(pair) == lend);
  assert(disc_pair_lori(pair) == lori);
  assert(disc_pair_rchr(pair) == rchr);
  assert(disc_pair_rstart(pair) == rstart);
  assert(disc_pair_rend(pair) == rend);
  assert(disc_pair_rori(pair) == rori);

  assert(!gasv_signalp(pair));
  assert(disc_pairp(pair));
  assert(!conc_pairp(pair));
  pair->conc = 1;
  assert(!disc_pairp(pair));
  assert(conc_pairp(pair));
  pair->conc = 0;
  

  // PREDICATES

  // Earlier disc_pair's should have smaller comparison values.
  assert(0 <= disc_pair_cmp(pair2, pair3));
  assert(0 <= disc_pair_cmp(pair3, pair));
  assert(0 <= disc_pair_cmp(pair2, pair));
  

  // I/O

  FILE *out = file_open("tmp/disc_pair/disc_pair.gasv", "w");
  disc_pair_write(pair, out);
  fclose(out);

  FILE *in = file_open("tmp/disc_pair/disc_pair.gasv", "r");
  assert(0 < disc_pair_read(&pair4, in));
  fclose(in);

  assert(!disc_pair_cmp(pair, &pair4));
  assert(!strcmp(disc_pair_qname(pair), disc_pair_qname(&pair4)));
  assert(disc_pairp(&pair4));
  assert(!conc_pairp(&pair4));

  disc_pair_free(pair);
  disc_pair_free(pair2);
  disc_pair_free(pair3);
  disc_pair_clear(&pair4);  
  
  return 0;
}
