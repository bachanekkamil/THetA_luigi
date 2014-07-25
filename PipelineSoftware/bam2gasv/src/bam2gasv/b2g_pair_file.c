#include <stdio.h>
#include "io.h"
#include "b2g_libraries.h"
#include "b2g_variant_type.h"
#include "b2g_error.h"

FILE *_open_gasv_input_file(char *OUTPUT_PREFIX, b2g_libraries_t *libraries, int lib, b2g_variant_t vtype, int GASV_PRO, int SPLIT_BY_CHROMOSOME, int lchr, int rchr) {
  FILE *out = NULL;
  if (SPLIT_BY_CHROMOSOME) out = file_open("%s_%s_%d_%d.%s", "a", OUTPUT_PREFIX, (CONC == vtype && GASV_PRO) ? "all" : b2g_library_name(b2g_libraries_get(libraries, lib)), lchr, rchr, VARIANT_EXTENSIONS[vtype]);
  else out = file_open("%s_%s.%s", "a", OUTPUT_PREFIX, (CONC == vtype && GASV_PRO) ? "all" : b2g_library_name(b2g_libraries_get(libraries, lib)), VARIANT_EXTENSIONS[vtype]);
  if (!out) b2g_exit(B2G_ERR_CANNOT_OPEN_OUTPUT_FILES);
  return out;
}
