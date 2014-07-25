#include <assert.h>
#include "b2g_bam.h"
#include "b2g_bam_header.h"
#include "params2gasv_input.h"
#include "b2g_error.h"
#include "b2g_constants.h"
#include "bam.h"
#include "io.h"

#define NUM_STATS 4
#define MAX_FILENAME 4096
#define ori2num(ori) ('-' == (ori))

static char FNAME[MAX_FILENAME];

static char *_lowq_filename(char *OUTPUT_PREFIX) {
  sprintf(FNAME, "%s_lowqual.bam", OUTPUT_PREFIX);
  return FNAME;
}

static char *_splitread_filename(char *OUTPUT_PREFIX) {
  sprintf(FNAME, "%s_splitread.bam", OUTPUT_PREFIX);
  return FNAME;
}



static void _read_lib_data(char *OUTPUT_PREFIX, char lib_names[][MAX_LIB_NAME_SIZE], int lib_stats[][NUM_STATS]) {
  FILE *info_file = file_open("%s.info", "r", OUTPUT_PREFIX);
  char buff[4096];
  int line = 0;
  assert(info_file);
  fgets(buff, 4096, info_file); // Skip first line
  while (-1 < fscanf(info_file, "%s\t%d\t%d\t%d\t%d", lib_names[line], &lib_stats[line][0], &lib_stats[line][1], &lib_stats[line][2], &lib_stats[line][3])) line++;
  fclose(info_file);
}

static void _header_stats(char *BAM_PATH, int *NUM_CHROMOSOMES, int *NUM_READGROUPS, int *NUM_LIBRARIES, int *LIB_SEP, int PROPER_LENGTH) {
  samfile_t *bamfile = b2g_samfile_open(BAM_PATH, "rb", 0);
  b2g_bam_header_counts(bamfile->header, NUM_CHROMOSOMES, NUM_READGROUPS, NUM_LIBRARIES, LIB_SEP, PROPER_LENGTH);
  samclose(bamfile);
}

static int _count_reads(char *path) {
  int count = 0;
  samfile_t *bamfile = b2g_samfile_open(path, "rb", 0);
  if (!bamfile) return 0;
  bam1_t *bam = bam_init1();
  while (-1 < samread(bamfile, bam)) count++;
  bam_destroy1(bam);
  samclose(bamfile);
  return count;
}

static int _count_pairs(char *OUTPUT_PREFIX, char *lib, b2g_variant_t vtype) {
  int count = 0;
  char buff[4096];
  FILE *f = lib ? file_open("%s_%s.%s", "r", OUTPUT_PREFIX, lib, VARIANT_EXTENSIONS[vtype]) : file_open("%s_all.%s", "r", OUTPUT_PREFIX, VARIANT_EXTENSIONS[vtype]);
  if (!f) return 0;
  while (fgets(buff, 4096, f)) count++;
  fclose(f);
  return count;
}

static void _check_types_conc(char *fname, int lmin, int lmax) {
  FILE *f = fopen(fname, "r");
  int lchr, lstart, rend;
  while (-1 < fscanf(f, "%d\t%d\t%d\n", &lchr, &lstart, &rend)) assert(CONC == variant_type(lchr, lchr, 0, 1, lstart, rend, lmin, lmax));
  fclose(f);
}

static void _check_types_disc(char *fname, int lmin, int lmax) {
  FILE *f = fopen(fname, "r");
  if (!f) return;
  char *suffix = strrchr(fname, '.');
  char qname[MAX_QNAME_SIZE];
  int lchr, rchr, lstart, lend, rstart, rend;
  char lori, rori;
  int tmp;

  while (-1 < (tmp = fscanf(f, "%s\t%d\t%d\t%d\t%c\t%d\t%d\t%d\t%c\n", qname, &lchr, &lstart, &lend, &lori, &rchr, &rstart, &rend, &rori))) {
    switch (variant_type(lchr, rchr, ori2num(lori), ori2num(rori), lstart, rend, lmin, lmax)) {
    case INS:
      assert(!strcmp(".insertion", suffix));
      break;
    case DEL:
      assert(!strcmp(".deletion", suffix));
      break;
    case TRANS:
      assert(!strcmp(".translocation", suffix));
      break;
    case INV:
      assert(!strcmp(".inversion", suffix));
      break;
      case DIV:
      assert(!strcmp(".divergent", suffix));
      break;
    case NUM_VTYPES:
    case CONC:
      assert(0);
    }
  }
  fclose(f);
}

static void _check_types(char *OUTPUT_PREFIX) {
  char fname[MAX_FILENAME];
  int lmin, lmax;
  FILE *gasv_in = file_open("%s.gasv.in", "r", OUTPUT_PREFIX);
  while (-1 < fscanf(gasv_in, "%s\tPR\t%d\t%d\n", fname, &lmin, &lmax)) {
    if (!strcmp(".concordant", strrchr(fname, '.'))) _check_types_conc(fname, lmin, lmax);
    else _check_types_disc(fname, lmin, lmax);
  }
  fclose(gasv_in);
}

static void _check_quality(char *OUTPUT_PREFIX, int WRITE_LOWQ, int WRITE_SPLITREAD, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  bam1_t *b1 = bam_init1(), *b2 = bam_init1();;
  if (WRITE_SPLITREAD) {
    samfile_t *split_file = b2g_samfile_open("%s_splitread.bam", "rb", 0, OUTPUT_PREFIX);
    while (-1 < samread(split_file, b1)) {
      samread(split_file, b2);
      assert(b2g_bam_pair_split(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
    }
    samclose(split_file);
  }
  if (WRITE_LOWQ) {
    samfile_t *lowq_file = b2g_samfile_open("%s_lowqual.bam", "rb", 0, OUTPUT_PREFIX);
    while (-1 < samread(lowq_file, b1)) {
      samread(lowq_file, b2);
      if (WRITE_SPLITREAD) {
	assert(!b2g_bam_pair_split(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
	assert(!b2g_bams_highq(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
      }
      else assert(!b2g_bams_highq(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES) || b2g_bam_pair_split(b1, b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
    }
    samclose(lowq_file);
  }

  bam_destroy1(b1);
  bam_destroy1(b2);
}

static void _check_lowq_split_counts(char *OUTPUT_PREFIX, int WRITE_LOWQ, int WRITE_SPLITREAD, int *lowq_count, int *splitread_count, int AMBIGUOUS) {
  int lowqc = _count_reads(_lowq_filename(OUTPUT_PREFIX));
  int splitc = _count_reads(_splitread_filename(OUTPUT_PREFIX));
  assert(0 == lowqc % 2);
  assert(0 == splitc % 2);
  if (AMBIGUOUS) {
    assert(0 == lowqc);
    assert(0 == splitc);
  }
  else { // !AMBIGUOUS
    if (-1 == *lowq_count || -1 == *splitread_count) {
      assert(WRITE_LOWQ  && WRITE_SPLITREAD);
      *lowq_count = lowqc;
      *splitread_count = splitc;
    }

    if (WRITE_LOWQ && WRITE_SPLITREAD) {
      assert(*lowq_count == lowqc);
      assert(*splitread_count == splitc);
    }
    else if (WRITE_LOWQ && !WRITE_SPLITREAD) {
      assert(lowqc == *lowq_count + *splitread_count);
      assert(0 == splitc);
    }
    else if (!WRITE_LOWQ && WRITE_SPLITREAD) {
      assert(splitc == *splitread_count);
      assert(0 == lowqc);
    }
    else {
      assert(0 == lowqc);
      assert(0 == splitc);
    }
  }
}

static void _check_all_counts(char *OUTPUT_PREFIX, int LIB_SEP, int WRITE_CONCORDANT, int GASV_PRO, int all_counts[NUM_VTYPES]) {
  int i, tmp_all_counts[NUM_VTYPES];
  for (i=0; i<NUM_VTYPES; i++) tmp_all_counts[i] = _count_pairs(OUTPUT_PREFIX, NULL, i);

  if (-1 == all_counts[0]) {
    assert(!LIB_SEP && (WRITE_CONCORDANT || GASV_PRO)); // Must start with all the all counts.
    for (i=0; i<NUM_VTYPES; i++) all_counts[i] = tmp_all_counts[i];
  }

  for (i=0; i<CONC; i++) {
    if (!LIB_SEP) assert(all_counts[i] == tmp_all_counts[i]);
    else assert(0 == tmp_all_counts[i]);
  }
  if ((!LIB_SEP && WRITE_CONCORDANT) || GASV_PRO) {
    assert(all_counts[CONC] == tmp_all_counts[CONC]);
  }
  else assert(0 == tmp_all_counts[CONC]);
}

static void _check_lib_counts(char *OUTPUT_PREFIX, int LIB_SEP, int WRITE_CONCORDANT, int GASV_PRO, int all_counts[NUM_VTYPES], int lib_counts[][NUM_VTYPES], int num_libs) {
  int i, j, sum, tmp_lib_counts[num_libs][NUM_VTYPES];
  char lib_names[num_libs][MAX_LIB_NAME_SIZE];
  int lib_stats[num_libs][NUM_STATS];
  _read_lib_data(OUTPUT_PREFIX, lib_names, lib_stats);

  if (!LIB_SEP) return;
  assert(-1 != all_counts[0]);

  for (i=0; i<num_libs; i++) {
    for (j=0; j<NUM_VTYPES; j++) {
      tmp_lib_counts[i][j] = _count_pairs(OUTPUT_PREFIX, lib_names[i], j);
    }
  }

  if (-1 == lib_counts[0][0]) {
    assert(LIB_SEP && !GASV_PRO && WRITE_CONCORDANT);
    for (i=0; i<num_libs; i++) {
      for (j=0; j<NUM_VTYPES; j++) {
	lib_counts[i][j] = tmp_lib_counts[i][j];
      }
    }
  }

  for (j=0; j<NUM_VTYPES; j++) {
    sum = 0;
    for (i=0; i<num_libs; i++) {
      sum += tmp_lib_counts[i][j];
      if (j != CONC || (WRITE_CONCORDANT && !GASV_PRO)) assert(lib_counts[i][j] == tmp_lib_counts[i][j]);
      else assert(0  == tmp_lib_counts[i][j]);
    }
    if (j != CONC || (WRITE_CONCORDANT && !GASV_PRO)) assert(sum == all_counts[j]);
    else assert(sum == 0);
  }
}

static void _check_pair_counts(char *OUTPUT_PREFIX, int LIB_SEP, int WRITE_CONCORDANT, int GASV_PRO, int all_counts[NUM_VTYPES], int lib_counts[][NUM_VTYPES], int num_libs) {
  _check_all_counts(OUTPUT_PREFIX, LIB_SEP, WRITE_CONCORDANT, GASV_PRO, all_counts);
  _check_lib_counts(OUTPUT_PREFIX, LIB_SEP, WRITE_CONCORDANT, GASV_PRO, all_counts, lib_counts, num_libs);
}

static void _check_total_counts(int all_counts[NUM_VTYPES], int *lowq_counts, int *splitread_counts, int total_reads, int LIB_SEP, int WRITE_LOWQ, int WRITE_CONCORDANT, int GASV_PRO, unsigned int num_skipped, int AMBIGUOUS) {
  int i, sum = *lowq_counts + *splitread_counts + num_skipped;
  if (!LIB_SEP && WRITE_LOWQ && (WRITE_CONCORDANT || GASV_PRO)) {
    for (i=0; i<NUM_VTYPES; i++) {
      sum += 2 * all_counts[i];
    }
    if (!AMBIGUOUS) assert(sum == total_reads);
  }
}

static void _check_counts(char *OUTPUT_PREFIX, int LIB_SEP, int WRITE_CONCORDANT, int GASV_PRO, int WRITE_LOWQ, int WRITE_SPLITREAD, int *lowq_count, int *splitread_count, int all_counts[NUM_VTYPES], int lib_counts[][NUM_VTYPES], int total_reads, unsigned int num_skipped, int num_libs, int AMBIGUOUS) {
  _check_lowq_split_counts(OUTPUT_PREFIX, WRITE_LOWQ, WRITE_SPLITREAD, lowq_count, splitread_count, AMBIGUOUS);
  _check_pair_counts(OUTPUT_PREFIX, LIB_SEP, WRITE_CONCORDANT, GASV_PRO, all_counts, lib_counts, num_libs);
  _check_total_counts(all_counts, lowq_count, splitread_count, total_reads, LIB_SEP, WRITE_LOWQ, WRITE_CONCORDANT, GASV_PRO, num_skipped, AMBIGUOUS);
}

int main(int argc, char **argv) {

  char *BAM_PATH = "test/tcga.sorted.bam";
  char *OUTPUT_PREFIX = "tmp/integration/validate/Example";
  int WRITE_LOWQ = 0;
  int LIB_SEP = 1;
  int MAPPING_QUALITY = 10;
  int WRITE_CONCORDANT = 0;
  int LOW_MEMORY = 0;
  int AMBIGUOUS = 0;
  int VERBOSE = 0;
  int USE_NUMBER_READS = 500000;
  int PROPER_LENGTH = 10000;
  //t CUTOFF_MODE = FILENAME;
  int CUTOFF_X = 112;
  int CUTOFF_Y = 401;
  //ar *CUTOFF_NAME = "test/tcga_cutoffs"
  int DEBUG_LEVEL = 0;
  int WRITE_SPLITREAD = 0;
  int MIN_ALIGNED_PCT = 95;
  char *CHROMOSOME_NAMING = "test/tcga_chrnames";
  b2g_platform_t PLATFORM = SOLID;
  int VALIDATION_STRINGENCY = 0;
  int GASV_PRO = 1;
  int IGNORE_DUPLICATES = 1;
  int QNAME_SORTED = 0;
  int SPLIT_BY_CHROMOSOME = 0;
  unsigned int num_skipped = 0;

  int CUTOFF_MODES[] = {EXACT, FILENAME}; // Indexed by LIB_SEP.
  char *CUTOFF_NAMES[] = {NULL, "test/tcga_cutoffs"}; // Indexed by LIB_SEP.

  MAX_BAM_FILES = 2;

  int NUM_CHROMOSOMES, NUM_READGROUPS, NUM_LIBRARIES, TMP_LIB_SEP = 1;

  _header_stats(BAM_PATH, &NUM_CHROMOSOMES, &NUM_READGROUPS, &NUM_LIBRARIES, &TMP_LIB_SEP, PROPER_LENGTH);

  // Temp data allocations
  int total_reads = _count_reads(BAM_PATH),
    lowq_counts[NUM_PLATFORMS],
    splitread_counts[NUM_PLATFORMS],
    all_counts[NUM_PLATFORMS][NUM_VTYPES],
    lib_counts[NUM_PLATFORMS][NUM_LIBRARIES][NUM_VTYPES];
  memset(lowq_counts, -1, NUM_PLATFORMS * sizeof(int));
  memset(splitread_counts, -1, NUM_PLATFORMS * sizeof(int));
  memset(all_counts, -1, NUM_PLATFORMS * NUM_VTYPES * sizeof(int));
  memset(lib_counts, -1, NUM_PLATFORMS * NUM_LIBRARIES * NUM_VTYPES * sizeof(int));


  for (PLATFORM=0; PLATFORM<NUM_PLATFORMS; PLATFORM++) {
    for (AMBIGUOUS=0; AMBIGUOUS<2; AMBIGUOUS++) {
      for (LOW_MEMORY=0; LOW_MEMORY <= 1; LOW_MEMORY++) {
	for (QNAME_SORTED=0; QNAME_SORTED <= 1; QNAME_SORTED++) {
	  for (LIB_SEP=0; LIB_SEP <= 1; LIB_SEP++) {
	    if (!TMP_LIB_SEP && LIB_SEP) continue; // Skip lib sep if the BAM has no libraries
	    for (WRITE_CONCORDANT=1; 0 <= WRITE_CONCORDANT; WRITE_CONCORDANT--) {
	      for (GASV_PRO=0; GASV_PRO<=1; GASV_PRO++) {
		for (WRITE_LOWQ=1; 0 <= WRITE_LOWQ; WRITE_LOWQ--) {
		  for (WRITE_SPLITREAD=1; 0 <= WRITE_SPLITREAD; WRITE_SPLITREAD--) {


		    //QNAME_SORTED = LOW_MEMORY = LIB_SEP = WRITE_CONCORDANT = GASV_PRO =0;
		    //WRITE_LOWQ = WRITE_SPLITREAD = 1;

		    
		    //printf("PLATFORM: %d, AMBIGUOUS: %d, LOW_MEMORY: %d, QNAME_SORTED: %d, LIB_SEP: %d, WRITE_CONCORDANT: %d, GASV_PRO: %d, WRITE_LOWQ: %d, WRITE_SPLITREAD: %d\n", PLATFORM, AMBIGUOUS, LOW_MEMORY, QNAME_SORTED, LIB_SEP, WRITE_CONCORDANT, GASV_PRO, WRITE_LOWQ, WRITE_SPLITREAD);
		    system("rm -f tmp/integration/validate/*");
		    num_skipped = 0;

		    assert(B2GERR_NO_ERROR == b2g_error_print(params2gasv_input(BAM_PATH, OUTPUT_PREFIX, MAPPING_QUALITY, WRITE_CONCORDANT, WRITE_LOWQ, LOW_MEMORY, AMBIGUOUS, LIB_SEP, VERBOSE, CUTOFF_X, CUTOFF_Y, CUTOFF_MODES[LIB_SEP], CUTOFF_NAMES[LIB_SEP], PROPER_LENGTH, USE_NUMBER_READS, DEBUG_LEVEL, WRITE_SPLITREAD, MIN_ALIGNED_PCT, CHROMOSOME_NAMING, PLATFORM, VALIDATION_STRINGENCY, GASV_PRO, IGNORE_DUPLICATES, QNAME_SORTED, SPLIT_BY_CHROMOSOME, &num_skipped)));

		    _check_counts(OUTPUT_PREFIX, LIB_SEP, WRITE_CONCORDANT, GASV_PRO, WRITE_LOWQ, WRITE_SPLITREAD, &lowq_counts[PLATFORM], &splitread_counts[PLATFORM], all_counts[PLATFORM], lib_counts[PLATFORM], total_reads, num_skipped, NUM_LIBRARIES, AMBIGUOUS);
		    _check_types(OUTPUT_PREFIX);
		    _check_quality(OUTPUT_PREFIX, WRITE_LOWQ, WRITE_SPLITREAD,  MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES);
		  }
		}
	      }
	    }
	  }
	}
      }
    }
  }

  return 0;
}
