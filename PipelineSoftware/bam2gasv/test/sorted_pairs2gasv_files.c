#include <stdio.h>
#include "sorted_pairs2gasv_files.h"
#include "conc_pair.h"
#include "disc_pair.h"
#include "constants.h"

int main() {

  FILE *conc_out = fopen("tmp/sorted_pairs2gasv_files/conc", "w"); 
  list_t *concs = list(3, conc_pair(24, 0, 2147483647), conc_pair(1, 10, 100), conc_pair(1, 10, 1000));

  
  FILE *disc_out = fopen("tmp/sorted_pairs2gasv_files/disc", "w"); 

  char *pair_name = malloc(11 * sizeof(char));
  strcpy(pair_name, "pair_qname");
  char *pair_name2 = malloc(12 * sizeof(char));
  strcpy(pair_name2, "pair_qname2");
  char *pair_name3 = malloc(12 * sizeof(char));
  strcpy(pair_name3, "pair_qname3");
  int lchr = 0, lstart = 1, lend = 10, lori = 0, rchr = 24, rstart = 14, rend = 22, rori = 1;

  list_t *discs = list(3, disc_pair(pair_name, 0, lchr, lstart, lend, lori, rchr, rstart, rend, rori), disc_pair(pair_name2, 0, lchr + 1, lstart, lend, lori, rchr, rstart, rend, rori), disc_pair(pair_name3, 0, lchr, lstart, lend, lori, rchr, rstart, rend + 1, rori));


  sorted_pairs2gasv_files(concs, conc_out);
  sorted_pairs2gasv_files(discs, disc_out);

  
  list_free(concs, NULL);
  list_free(discs, NULL);
  fclose(conc_out);
  fclose(disc_out);
  return 0;
}
