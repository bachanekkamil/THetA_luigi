#include <assert.h>
#include <math.h>
#include <stdio.h>
#include "bam.h"
#include "b2g_libraries.h"
#include "b2g_cutoff_lminlmax.h"
#include "b2g_bam.h"
#include "b2g_pair.h"
#include "io.h"
#include "b2g_variant_type.h"
#include "b2g_constants.h"
#include "b2g_chromosome.h"
#include "b2g_compressed_bam.h"

// I/O


// Get the library index corresponding to the readgroup.
int _indexof_readgroup(b2g_libraries_t *libraries, char *readgroup) {
  int i;
  for (i=0; i<libraries->readgroups2libraries_length; i++) {
    if (!strcmp(libraries->readgroups2libraries[i].name, readgroup)) {
      return libraries->readgroups2libraries[i].library_index;
    }
  }
  return 0;
}

// Creates a pair and writes it to the correct library.
static void _write_pair(b2g_disc_pair_t *pair, b2g_bam_t *bam1, list_t *lib2vtype2unsorted_pairs, b2g_libraries_t *libraries, b2g_platform_t PLATFORM, int PROPER_LENGTH, int USE_NUMBER_READS, int WRITE_CONCORDANT, int GASV_PRO) {
  if (!pair) return;
  int library_index = _indexof_readgroup(libraries, b2g_bam_readgroup(bam1));    
  b2g_library_t *library = b2g_libraries_get(libraries, library_index);
  b2g_library_write(library, (b2g_pair_t *)pair, lib2vtype2unsorted_pairs + (library_index * NUM_VTYPES), PROPER_LENGTH, USE_NUMBER_READS, WRITE_CONCORDANT, PLATFORM, GASV_PRO);
}

void b2g_libraries_write_ambiguous(b2g_libraries_t *libraries, b2g_bam_t *bam1, list_t *bam2s, list_t *lib2vtype2unsorted_pairs, buffer_t *lowq_pairs, b2g_chromosomes_t *chromosomes, int MAPPING_QUALITY, int PROPER_LENGTH, int WRITE_CONCORDANT, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int VERBOSE, int USE_NUMBER_READS, int GASV_PRO) {

  if (!bam2s) return;
  int total, bam1_fop = b2g_bam_first_of_pair(bam1), second_first[2] = {};
  list_t *bam2s_counts = bam2s;
  char ambig_qname[MAX_QNAME_SIZE];

  // Count up the number of first, second, and total reads,
  while (bam2s_counts) {
    if (b2g_bam_first_of_pair(((b2g_bam_t *)bam2s_counts->car))) second_first[1]++;
    else second_first[0]++;
    bam2s_counts = bam2s_counts->cdr;
  }
  total = second_first[0] * second_first[1]; // Cartesion product: * not +

  // then walk through the existing reads, pairing bam1 with 
  while (bam2s) {
    // appropriate bam2s, and 
    if (bam1_fop != b2g_bam_first_of_pair((b2g_bam_t *)bam2s->car)) {
      // naming accordingly.
      sprintf(ambig_qname, "%s_%d_%d_%d", b2g_bam_qname(bam1), total++, 
	      bam1_fop ? second_first[1] : --second_first[1], // print either the number of the read or the decreasing index of its mate.
	      bam1_fop ? --second_first[0] : second_first[0]);
      
      if (b2g_bams_highq(bam1, bam2s->car, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) {    
	_write_pair(b2g_bams2disc_pair(bam1, bam2s->car, ambig_qname, chromosomes, PLATFORM), bam1, lib2vtype2unsorted_pairs, libraries, PLATFORM, PROPER_LENGTH, USE_NUMBER_READS, WRITE_CONCORDANT, GASV_PRO);
      }
    }
    bam2s = bam2s->cdr;
  }
}

void b2g_libraries_write_compressed(b2g_libraries_t *libraries, char *qname, b2g_bam_t *bam1, b2g_compressed_bam_t *bam2, list_t *lib2vtype2unsorted_pairs, b2g_chromosomes_t *chromosomes, int MAPPING_QUALITY, int PROPER_LENGTH, int WRITE_CONCORDANT, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int VERBOSE, int USE_NUMBER_READS, int GASV_PRO) {

  if (!bam2) return;

  if (b2g_bam_compressed_bam_highq(bam1, bam2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) {
    _write_pair(b2g_bam2disc_pair(bam1, qname, chromosomes, PLATFORM), bam1, lib2vtype2unsorted_pairs, libraries, PLATFORM, PROPER_LENGTH, USE_NUMBER_READS, WRITE_CONCORDANT, GASV_PRO);
  }

  b2g_compressed_bam_free(bam2);
}

void b2g_libraries_write(b2g_libraries_t *libraries, char *qname, b2g_bam_t *bam1, b2g_bam_t *bam2, list_t *lib2vtype2unsorted_pairs, buffer_t *lowq_pairs, b2g_chromosomes_t *chromosomes, int WRITE_LOWQ, int MAPPING_QUALITY, int PROPER_LENGTH, int WRITE_CONCORDANT, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES, b2g_platform_t PLATFORM, int VERBOSE, int USE_NUMBER_READS, int GASV_PRO, int WRITE_SPLITREAD) {
  assert(libraries && bam1 && lowq_pairs && is_bool(WRITE_LOWQ) && in_range(MAPPING_QUALITY, 0, 255) && is_nonnegative(PROPER_LENGTH) &&  is_bool(WRITE_CONCORDANT) && in_range(MIN_ALIGNED_PCT, 50, 100) && is_bool(IGNORE_DUPLICATES) && is_bool(VERBOSE) && is_positive(USE_NUMBER_READS) && is_bool(GASV_PRO) && is_bool(WRITE_SPLITREAD));

  if (!bam2) return;
  
  if (b2g_bams_highq(bam1, bam2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) {    
    _write_pair(b2g_bam2disc_pair(bam1, qname, chromosomes, PLATFORM), bam1, lib2vtype2unsorted_pairs, libraries, PLATFORM, PROPER_LENGTH, USE_NUMBER_READS, WRITE_CONCORDANT, GASV_PRO);
  }
  else if (WRITE_LOWQ || WRITE_SPLITREAD){ // If the pair is split/lowquality and we are writing lowqs, write it out.
    bam_copy1(buffer_buffer(lowq_pairs), (const bam1_t *)bam1);
    buffer_flush(lowq_pairs);
    bam_copy1(buffer_buffer(lowq_pairs), (const bam1_t *)bam2);
    buffer_flush(lowq_pairs);
  }
  b2g_bam_free(bam2);
}

void b2g_libraries_flush(b2g_libraries_t *libraries, list_t *lib2vtype2unsorted_pairs, int PROPER_LENGTH, int WRITE_CONCORDANT, b2g_platform_t PLATFORM, int VERBOSE) {
  assert(libraries && is_nonnegative(PROPER_LENGTH) && is_bool(WRITE_CONCORDANT) && is_bool(VERBOSE));
  int i;
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    b2g_library_flush(b2g_libraries_get(libraries, i), lib2vtype2unsorted_pairs + (i * NUM_VTYPES), WRITE_CONCORDANT, PLATFORM);
  }
}

void b2g_libraries_write_gasv_in(b2g_libraries_t *libraries, FILE *out, char *OUTPUT_PREFIX) {
  assert(libraries && out && OUTPUT_PREFIX);
  int i, j;
  for (i=0; i<b2g_libraries_length(libraries); i++) {    
    b2g_variant_t vtype[4] = {DEL, INV, DIV, TRANS};
    
    for (j=0; j<4; j++) {
      b2g_library_t *library = b2g_libraries_get(libraries, i);
      fprintf(out, "%s_%s.%s\t%s\t%d\t%d\n", OUTPUT_PREFIX, b2g_library_name(library), VARIANT_EXTENSIONS[vtype[j]], "PR", b2g_library_lmin(library), b2g_library_lmax(library));
    }  
  }
}


void b2g_libraries_write_info(b2g_libraries_t *libraries, FILE *out, int USE_NUMBER_READS, int VERBOSE) {
  assert(libraries && out && is_positive(USE_NUMBER_READS));
  int lib_idx;
  fprintf(out, "LibraryName\tLmin\tLmax\tMean\tStdDev\n");
  for (lib_idx=0; lib_idx<b2g_libraries_length(libraries); lib_idx++) {    
    b2g_library_t *library = b2g_libraries_get(libraries, lib_idx);
    b2g_library_stats_t *stats = &library->cutoff_data;
    if (!b2g_library_non_empty(library)) continue;
    fprintf(out, "%s\t%d\t%d\t%lu\t%lu\n", b2g_library_name(library),  b2g_library_stats_lmin(stats), b2g_library_stats_lmax(stats), (unsigned long)roundf(b2g_library_stats_mean(stats)), (unsigned long)roundf(b2g_library_stats_std(stats)));
  }      
}


void b2g_libraries_write_gasvpro_in(b2g_libraries_t *libraries, FILE *out, char *OUTPUT_PREFIX, int USE_NUMBER_READS, unsigned long GENOME_LENGTH) {
    assert(libraries && out && OUTPUT_PREFIX && is_positive(USE_NUMBER_READS) && is_positive(GENOME_LENGTH));
  int i;
  fprintf(out, "# Generated by BamToGASV\n");
  fprintf(out, "ConcordantFile: %s_all.concordant\n", OUTPUT_PREFIX);

  double total_conc = 0, total_conc_readlength = 0, total_conc_insertlength = 0;
  for (i=0; i<b2g_libraries_length(libraries); i++) {
    b2g_library_t *library = b2g_libraries_get(libraries, i);
    total_conc_readlength += b2g_library_total_concordant_read(library);
    total_conc_insertlength += b2g_library_total_concordant_insert(library);
    total_conc += b2g_library_total_concordant(library);
  }

  fprintf(out, "Lavg: %f\n", total_conc ? total_conc_insertlength / total_conc : 0);
  fprintf(out, "ReadLen: %f\n", total_conc ? total_conc_readlength / total_conc : 0);
  fprintf(out, "Lambda: %f\n", total_conc_insertlength / (double)GENOME_LENGTH);
}


