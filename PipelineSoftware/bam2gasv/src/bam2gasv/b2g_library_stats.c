#include <math.h>
#include "b2g_library_stats.h"
#include "b2g_cutoff_lminlmax.h"


// _COMPUTE HELPERS

// Compute standard deviation.
static double _compute_std(b2g_library_stats_t *stats) {
  double err, total_sq_err = 0, mean = stats->mean;
  list_t *pairs = stats->sample_pairs;

  while (pairs) {
    err = mean - b2g_disc_pair_insert_len((b2g_disc_pair_t *)pairs->car);
    total_sq_err += err * err;
    pairs = pairs->cdr;
  }
  return sqrt(total_sq_err / stats->total_pairs);
}

// Compute cutoff_lminlmax assuming PCT mode.
static void _compute_lminlmax_pct(b2g_library_stats_t *stats, int pct) { 
  assert(in_range(pct, 50, 100));

  list_t *pairs = stats->sample_pairs;
  unsigned int min_insert = stats->min_insert; // Lowest insert length in samples.
  unsigned int num_lengths = stats->max_insert - min_insert + 1;
  unsigned int insert_lengths[num_lengths];
  bzero(insert_lengths, sizeof(unsigned int) * num_lengths);
  
  // Fill a table of insert_length -> number_seen,
  while (pairs) {
    insert_lengths[b2g_disc_pair_insert_len((b2g_disc_pair_t *)pairs->car) - min_insert]++;
    pairs = pairs->cdr;
  }

  int i, total_insert = 0;
  // determine how many samples should fall under the min and max quantile,
  int min_quantile = floor(stats->total_pairs * ((double)(100 - pct) / 100.0));
  int max_quantile = ceil(stats->total_pairs * ((double)pct / 100.0));

  // and count up the observed samples to find our quantile lengths.
  for (i=0; i<num_lengths; i++) {    
    if (total_insert < min_quantile) stats->lmin = i + min_insert;
    if (total_insert < max_quantile) stats->lmax = i + min_insert;
    total_insert += insert_lengths[i];
  }
}

static void _compute_lminlmax(b2g_library_stats_t *stats, b2g_cutoff_lminlmax_t cutoffs) {
switch (cutoffs.mode) {
  case EXACT:
    stats->lmin = cutoffs.X;
    stats->lmax = cutoffs.Y;
    break;
  case SD:
    stats->lmin = stats->mean - (cutoffs.X * stats->std);
    stats->lmax = stats->mean + (cutoffs.X * stats->std);
    break;
  case PCT:
    _compute_lminlmax_pct(stats, cutoffs.X);
    break;
  case FILENAME:
  case NO_MODE:
    assert(0);
    break;
  }
}

// Compute all statistics for observed samples.
static void _compute(b2g_library_stats_t *stats, b2g_cutoff_lminlmax_t cutoffs) {
  assert(stats && !stats->computed);
  stats->mean = stats->total_pairs ? stats->total_insert / stats->total_pairs : 0;
  stats->std = stats->total_pairs ? _compute_std(stats) : 0;
  if (stats->total_pairs) _compute_lminlmax(stats, cutoffs);
  else stats->lmin = stats->lmax = 0;
  stats->computed = 1;
}



// PAIR CLASSIFYING AND WRITING

static void _conc_pair2buffer(b2g_library_stats_t *stats, b2g_disc_pair_t *pair, list_t *unsorted_pairs, int WRITE_CONCORDANT) {
  stats->total_concordant++;
  stats->total_conc_read += b2g_disc_pair_readlength(pair);
  stats->total_conc_insert += b2g_disc_pair_insert_len(pair);

  if (WRITE_CONCORDANT) {
    list_buffer_write(unsorted_pairs, (b2g_pair_t *)b2g_disc_pair2b2g_conc_pair(pair));
  }

  b2g_disc_pair_free(pair);
}

// Classify the vtype of the pair and write it to the appropriate buffer.
static void _pair2buffer(b2g_library_stats_t *stats, b2g_disc_pair_t *pair, list_t vtype2unsorted_pairs[NUM_VTYPES], int WRITE_CONCORDANT, b2g_platform_t PLATFORM) {

  int vtype = b2g_pair_vtype((b2g_pair_t *)pair, stats->lmin, stats->lmax, PLATFORM);  
  if (CONC == vtype) _conc_pair2buffer(stats, pair, &vtype2unsorted_pairs[CONC], WRITE_CONCORDANT);
  else {
    list_buffer_write(&vtype2unsorted_pairs[vtype], pair);
  }
}

// Runs _pair2buffer on all samples and dispose of the list. This is used to flush the samples after stats have been computed.
static void _pairs2buffers(b2g_library_stats_t *stats, list_t vtype2unsorted_pairs[NUM_VTYPES], int WRITE_CONCORDANT, b2g_platform_t PLATFORM) {
  assert(stats && stats->computed);

  list_t *pairs = stats->sample_pairs;
  while (pairs) {
    _pair2buffer(stats, (b2g_disc_pair_t *)pairs->car, vtype2unsorted_pairs, WRITE_CONCORDANT, PLATFORM);
    pairs = pairs->cdr;
  }
  list_free(stats->sample_pairs, NULL);
  stats->sample_pairs = NULL;
}

// Pushes a pair into the sample set and update relevant statistics.
static void _pair2samples(b2g_library_stats_t *stats, b2g_disc_pair_t *pair) {
  stats->sample_pairs = cons(pair, stats->sample_pairs);

  unsigned int insert = b2g_disc_pair_insert_len(pair);
  if (insert < stats->min_insert) stats->min_insert = insert;
  if (stats->max_insert < insert) stats->max_insert = insert;
  stats->total_insert += insert;
  stats->total_read += b2g_disc_pair_readlength(pair);
  stats->total_pairs++;
}



void b2g_library_stats_write(b2g_library_stats_t *stats, b2g_pair_t *pair, list_t vtype2unsorted_pairs[NUM_VTYPES], b2g_cutoff_lminlmax_t cutoffs, int PROPER_LENGTH, int USE_NUMBER_READS, int WRITE_CONCORDANT, b2g_platform_t PLATFORM, int GASV_PRO) {
  assert(stats && pair && is_nonnegative(PROPER_LENGTH) && is_positive(USE_NUMBER_READS) && is_bool(WRITE_CONCORDANT));

  stats->non_empty = 1;

  // If we do not need further sampling to classify this pair,
  if (stats->computed ||
      !(b2g_pair_equal_chr(pair) && b2g_pair_convergent(pair)) ||
      (PROPER_LENGTH && PROPER_LENGTH < b2g_disc_pair_insert_len((b2g_disc_pair_t *)pair))) {
    // classify it. 
    _pair2buffer(stats, (b2g_disc_pair_t *)pair, vtype2unsorted_pairs, WRITE_CONCORDANT, PLATFORM);
  }
  else {    
    // Otherwise, add it to the samples and,
    _pair2samples(stats, (b2g_disc_pair_t *)pair);

    // if we have reached our sample quota,
    if (USE_NUMBER_READS == stats->total_pairs) {
      // compute stats and flush.
      b2g_library_stats_flush(stats, cutoffs, vtype2unsorted_pairs, WRITE_CONCORDANT, PLATFORM);
    }
  }
}
 
void b2g_library_stats_flush(b2g_library_stats_t *stats, b2g_cutoff_lminlmax_t cutoffs, list_t vtype2unsorted_pairs[NUM_VTYPES], int WRITE_CONCORDANT, b2g_platform_t PLATFORM) {
  assert(stats && is_bool(WRITE_CONCORDANT));
  if (stats->computed) return; // Flush is idempotent.
  _compute(stats, cutoffs);      
  _pairs2buffers(stats, vtype2unsorted_pairs, WRITE_CONCORDANT, PLATFORM);
}
