// Provides a datatype representing all of the statistics collected about a b2g_library. Used for estimating lmax and lmin, and for generating GASVPRO parameters.

#ifndef B2G_LIBRARY_STATS_H
#define B2G_LIBRARY_STATS_H

#include "b2g_cutoff_lminlmax.h"
#include "list.h"
#include "b2g_pair.h"

// DATATYPES

// A collection of statistics belonging to a single library.
typedef struct { 
  // ordered for initialization purposes
  int lmax; // Starts at PROPER_LENGTH to allow writing of improper pairs. Estimated later from cutoff_lminlmax parameters.
  unsigned int min_insert; // Minimum insert length: starts at max int (-1) and is recomputed as lower insert lengths are seen.

  // unordered (all 0/NULL)
  unsigned int max_insert; // Maximum insert length. 
  int lmin;
  list_t *sample_pairs; // List of pairs used for estimating lmin/lmax.
  double total_pairs; // Total number of samples seen thus far. Incremented on every new sample and used to check when we have enough samples to compute the rest of the statistics.  
  double total_insert; // Total insert length of all samples.
  double total_read; // Total read length of all samples.
  double mean; // Average sample insert length.
  double std; // Standard deviation of sample insert lengths.
  double total_concordant; // Total number of concordant pairs seen throughout entire lifetime of library (beyond only the samples used for computing).
  double total_conc_read; // This is the true total concordant readlength, in case we want to replace the approximate method from the old algorithm.
  double total_conc_insert; // This is the true total concordant insert length, in case we want to replace the approximate method from the old algorithm.
  int computed; // Boolean flag. True if the stats have been computed for this library.
  int non_empty; // True if library has at least one
} b2g_library_stats_t;


// ACCESSORS

#define b2g_library_stats_mean(stats) ((stats)->mean)
#define b2g_library_stats_std(stats) ((stats)->std)
#define b2g_library_stats_lmin(stats) ((stats)->lmin)
#define b2g_library_stats_lmax(stats) ((stats)->lmax)
#define b2g_library_stats_total_pairs(stats) ((stats)->total_pairs)
#define b2g_library_stats_non_empty(stats) ((stats)->non_empty)

// Statistics about total concordant pairs seen are estimated based on statistics on the sample_pairs and the number of concordants seen in total.
#define b2g_library_stats_total_concordant(stats) ((stats)->total_concordant)
//#define b2g_library_stats_total_concordant_read(stats) ((stats)->total_concordant * ((stats)->total_read / (stats)->total_pairs))
#define b2g_library_stats_total_concordant_read(stats) ((stats)->total_conc_read)
//#define b2g_library_stats_total_concordant_insert(stats) ((stats)->total_concordant * ((stats)->total_insert / (stats)->total_pairs))
#define b2g_library_stats_total_concordant_insert(stats) ((stats)->total_conc_insert)

// I/O

// Accepts a pair, classifies its type, and pushes it into the appropriate buffer. The actual write may be delayed if still collecting samples for computing stats. Returns true if GASV_PRO is merging concordants, stats have been computed, and the pair is concordant, signaling that the pair should be redirected to the "all" concordant library.
void b2g_library_stats_write(b2g_library_stats_t *, b2g_pair_t *, list_t [NUM_VTYPES], b2g_cutoff_lminlmax_t, int, int, int, b2g_platform_t, int);

// Force the computation of stats and flush any current samples to the appropriate buffers. This is only necessary when there are not enough pairs to meet our quota of samples.
void b2g_library_stats_flush(b2g_library_stats_t *, b2g_cutoff_lminlmax_t , list_t [NUM_VTYPES], int, b2g_platform_t);


#endif
