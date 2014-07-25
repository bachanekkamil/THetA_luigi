#include "b2g_library.h"
#include "b2g_library_stats.h"
#include "b2g_constants.h"

// CONSTRUCTORS

void b2g_library_init(b2g_library_t *library, const char *name, b2g_cutoff_lminlmax_t *cutoff_lminlmax, int PROPER_LENGTH) {
  assert(library && name && cutoff_lminlmax && is_nonnegative(PROPER_LENGTH));

  b2g_library_stats_t stats = {PROPER_LENGTH, -1};
  strcpy(library->name, name);
  library->cutoff_lminlmax = *cutoff_lminlmax;
  library->cutoff_data = stats;
} 

b2g_error_t b2g_library_cutoff_lminlmax_read(FILE *cutoff_file, b2g_library_t libraries[], int num_libraries, int VERBOSE) {
 
  if (!cutoff_file) return B2GERR_NO_ERROR; // If no file is supplied, we succeed with no change.
  
  int i, X, Y;
  int ERR;
  b2g_cutoff_lminlmax_t default_cutoff = {PCT, 99, -1, NULL};
  b2g_cutoff_lminlmax_mode_t mode;
  char name[MAX_LIB_NAME_SIZE];  

  // For each line of the cutoff_lminlmax file,
 next_line: while (!(ERR = b2g_cutoff_lminlmax_read(cutoff_file, &mode, &X, &Y, name))) {    
    for (i=0; i<num_libraries; i++) {
      // if we have seen its library before,
      if (!strcmp(libraries[i].name, name)) {
	// overwrite the default cutoff mode with the mode from the file.
	b2g_cutoff_lminlmax_t cutoff = {mode, X, Y, name};
	libraries[i].cutoff_lminlmax = cutoff;
	goto next_line;
      }
    }
    // If there was no match, error.
    return B2GERR_LIBRARY_NOT_FOUND;
  }
  
  for (i=0; i<num_libraries; i++) {
    if (VERBOSE && FILENAME == libraries[i].cutoff_lminlmax.mode) {
      printf("Library %s not found in -CUTOFF_LMINLMAX file: setting cutoff mode to default PCT=99%%\n\n", b2g_library_name(&libraries[i]));
      libraries[i].cutoff_lminlmax = default_cutoff;
    }
  }

  if (-2 == ERR) return B2GERR_CUTOFF_FILE_INVALID_FORMAT; // Abort if there was an invalid read.
  else return B2GERR_NO_ERROR;
}


// I/O

//TODO make this void
void b2g_library_write(b2g_library_t *library, b2g_pair_t *pair, list_t vtype2unsorted_pairs[NUM_VTYPES], int PROPER_LENGTH, int USE_NUMBER_READS, int WRITE_CONCORDANT, b2g_platform_t PLATFORM, int GASV_PRO) {
  b2g_library_stats_write(&library->cutoff_data, pair, vtype2unsorted_pairs, library->cutoff_lminlmax, PROPER_LENGTH, USE_NUMBER_READS, WRITE_CONCORDANT, PLATFORM, GASV_PRO);
}

void b2g_library_flush(b2g_library_t *library, list_t vtype2unsorted_pairs[NUM_VTYPES], int WRITE_CONCORDANT, b2g_platform_t PLATFORM) {
  b2g_library_stats_flush(&library->cutoff_data, library->cutoff_lminlmax, vtype2unsorted_pairs, WRITE_CONCORDANT, PLATFORM);
}
