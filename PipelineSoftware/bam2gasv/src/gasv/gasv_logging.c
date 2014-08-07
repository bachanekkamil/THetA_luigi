#include "gasv_logging.h"
#include <stdio.h>
#include <stdlib.h>

void gasv_error(gasv_error_t err) {
  switch (err) {
  case GASV_ERR_MAX_LT_MIN:
    printf("ERROR: Lmin has been increased beyond >= Lmax for ESP. Halting program!\n");
    break;
  }
  exit(-1);
}
