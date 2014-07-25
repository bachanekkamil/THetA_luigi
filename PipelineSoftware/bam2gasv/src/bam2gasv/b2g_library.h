// Provides a datatype representing a single library from the input BAM file. Used to classify pairs based on lmin and lmax estimated from pairs in the same library.

#ifndef B2G_LIBRARY_H
#define B2G_LIBRARY_H

#include "b2g_cutoff_lminlmax.h"
#include "b2g_library_stats.h"
#include "buffer.h"

// DATATYPES

// A single library in a BAM file.
typedef struct {
  char name[MAX_LIBNAME_LENGTH]; // Library name.
  b2g_cutoff_lminlmax_t cutoff_lminlmax; // Library-specific cutoff_lminlmax parameters for estimating lmin and lmax.
  b2g_library_stats_t cutoff_data; //TODOrename to stats
} b2g_library_t;


// CONSTRUCTORS

void b2g_library_init(b2g_library_t *, const char *, b2g_cutoff_lminlmax_t *, int);

// Updates an array of libraries based on the cutoff_lminlmax parameters stored in a file.
b2g_error_t b2g_library_cutoff_lminlmax_read(FILE *, b2g_library_t [], int, int);


// ACCESSORS

#define b2g_library_name(lib) ((lib)->name)
#define b2g_library_lmin(lib) (b2g_library_stats_lmin(&(lib)->cutoff_data))
#define b2g_library_lmax(lib) (b2g_library_stats_lmax(&(lib)->cutoff_data))
#define b2g_library_total_pairs(lib) (b2g_library_stats_total_pairs(&(lib)->cutoff_data))
#define b2g_library_non_empty(lib) (b2g_library_stats_non_empty(&(lib)->cutoff_data))

// Statistics about total concordant pairs seen are estimated based on statistics on the sample_pairs and the number of concordants seen in total.
#define b2g_library_total_concordant(lib) (b2g_library_stats_total_concordant(&lib->cutoff_data))
#define b2g_library_total_concordant_read(lib) (b2g_library_stats_total_concordant_read(&lib->cutoff_data))
#define b2g_library_total_concordant_insert(lib) (b2g_library_stats_total_concordant_insert(&lib->cutoff_data))


// I/O

// Accepts a pair, classifies its type, and pushes it into the appropriate buffer. The actual write may be delayed if still collecting samples for computing stats. Returns 1 if pair is a concordant pair, 0 otherwise. Concordants is set to a list of concordants if this write triggered a flush, NULL otherwise.
void b2g_library_write(b2g_library_t *, b2g_pair_t *, list_t *, int, int, int, b2g_platform_t, int);

// This forces the write, with the preconditions that the pair is concordant, that this is the "all" concordant library, and that we are merging concordants in GASV_PRO mode.
void b2g_library_write_concordant(b2g_library_t *, b2g_pair_t *, buffer_t *);

// Force the computation of stats and flush any current samples to the appropriate buffers. This is only necessary when there are not enough pairs to meet our quota of samples.
void b2g_library_flush(b2g_library_t *, list_t [NUM_VTYPES], int, b2g_platform_t);

#endif
