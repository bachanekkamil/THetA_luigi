// Contains general values and datatypes too small to warrant their own modules.

#ifndef B2G_CONSTANTS_H
#define B2G_CONSTANTS_H

#include "list.h"
#include <assert.h>

// String representations for converting between internal representations and external.
extern char* BOOLS[];
extern char ORIENTATION_CHARS[];
extern char *PLATFORMS[];


// Types
typedef enum {ILLUMINA, SOLID, MATEPAIR, NUM_PLATFORMS} b2g_platform_t;

// Externs that can be tweaked dynamically ie for testing.
extern int MAX_BAM_FILES; 
extern int BAMS_READ_PER_OUTPUT;
extern int B2G_HASH_SIZE;

// Numeric constants.
#define MAX_FILENAME_SIZE 4096
#define MAX_LIB_NAME_SIZE 1024
#define MAX_QNAME_SIZE 1024
#define DEFAULT_GENOME_LENGTH 3000000000
#define MAX_CHRNAME_LENGTH 1024
#define MAX_LIBNAME_LENGTH 4096
#define MAX_READGROUP_LENGTH 4096
#define GASV_FILES2GASV_BUFFER_SIZE 1000
#define PAIR_BUFFER_SIZE 1000
#define SORTED_BUFFER_SIZE 1000
#define FILES2OUTPUT_BUFFER_SIZE 1000
#define BAM_FILES2BAMS_BUFFER 10000
#define LOWQ_BUFFER_SIZE 1000
#define MAX_LIBRARIES 100
#define MAX_BAMS_TO_READ 0


// TESTING UTILITIES

#define is_bool(b) (0 == (b) || 1 == (b))
#define is_positive(i) (0 < (i))
#define in_range(i, start, end) ((start) <= (i) && (i) <= (end))
#define is_nonnegative(i) (0 <= (i))

#endif
