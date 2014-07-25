#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <stdarg.h>
#include <assert.h>
#include "b2g_bam.h"
#include "sam_header.h"
#include "list.h"
#include "b2g_constants.h"
#include "io.h"
#include "str.h"
#include "bam.h"
#include "b2g_signal.h"
#include "str.h"
#include "b2g_chromosome.h"

#define CIGAR_SOFTCLIP 4


// Samtools' qname sorting cmp function
static int strnum_cmp(const char *_a, const char *_b)
{
	const unsigned char *a = (const unsigned char*)_a, *b = (const unsigned char*)_b;
	const unsigned char *pa = a, *pb = b;
	while (*pa && *pb) {
		if (isdigit(*pa) && isdigit(*pb)) {
			while (*pa == '0') ++pa;
			while (*pb == '0') ++pb;
			while (isdigit(*pa) && isdigit(*pb) && *pa == *pb) ++pa, ++pb;
			if (isdigit(*pa) && isdigit(*pb)) {
				int i = 0;
				while (isdigit(pa[i]) && isdigit(pb[i])) ++i;
				return isdigit(pa[i])? 1 : isdigit(pb[i])? -1 : (int)*pa - (int)*pb;
			} else if (isdigit(*pa)) return 1;
			else if (isdigit(*pb)) return -1;
			else if (pa - a != pb - b) return pa - a < pb - b? 1 : -1;
		} else {
			if (*pa != *pb) return (int)*pa - (int)*pb;
			++pa; ++pb;
		}
	}
	return *pa? 1 : *pb? -1 : 0;
}

// SAM FILES

samfile_t* b2g_samfile_open(char* path, char* mode, void* aux, ...) {
  char filename[1024] = {};
  va_list args;
  va_start(args, aux);  
  vsprintf(filename, path, args);
  va_end(args);

  if ('r' == mode[0] && access(filename, R_OK)) return NULL;

  samfile_t *in = samopen(filename, mode, aux);
  
  if (in) {
    if (!in->header) return NULL;
    // Make sure to parse the header for later use.
    bam_header_t *header = in->header;    
    if (header->dict == 0) header->dict = sam_header_parse2(header->text);
    if (header->rg2lib == 0) header->rg2lib = sam_header2tbl(header->dict, "RG", "ID", "LB");
  }
  return in;
}


// CONSTRUCTORS


b2g_bam_t *b2g_bam_clone(b2g_bam_t* bam) {
  b2g_bam_t* gbam = b2g_malloc(sizeof(b2g_bam_t));
  memcpy(gbam, bam, sizeof(b2g_bam_t));
  gbam->data = b2g_malloc(bam->data_len);
  memcpy(gbam->data, bam->data, bam->data_len);
  return gbam;
}


void b2g_bam_free(b2g_bam_t *b) {
  if (b && !b2g_signalp(b)) {    
    if (b->data) free(b->data);
    free(b);
  }
}

void b2g_bam_clear(b2g_bam_t *b) {
  if (b && !b2g_signalp(b) && b->data) free(b->data);
  bzero(b, sizeof(b2g_bam_t));
}

// PREDICATES

int b2g_bam_pair_split(b2g_bam_t *b1, b2g_bam_t *b2, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  b2g_bam_type_t t1 = b2g_bam_type(b1, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES);
  b2g_bam_type_t t2 = b2g_bam_type(b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES);
  return (HIGH == t1 && (UNMAPPED == t2 || SPLIT == t2)) ||
	  (HIGH == t2 && (UNMAPPED == t1 || SPLIT == t1));
}

int b2g_cmp(int lchr, int rchr, int lstart, int rstart, int lend, int rend) {
  if (lchr != rchr) return lchr - rchr;
  if (lstart != rstart) return lstart - rstart;
  return lend - rend;
}

int b2g_bam_cmp(b2g_bam_t *b1, b2g_bam_t *b2, b2g_chromosomes_t *chromosomes) {
  return b2g_cmp(b2g_bam_chr(b1, chromosomes),
		 b2g_bam_chr(b2, chromosomes),
		 b2g_bam_start(b1),
		 b2g_bam_start(b2),
		 b2g_bam_end(b1),
		 b2g_bam_end(b2));
}

int b2g_bam_mate_cmp(b2g_bam_t *b1, b2g_chromosomes_t *chromosomes) {
  return b2g_cmp(b2g_bam_chr(b1, chromosomes),
		 b2g_bam_mate_chr(b1, chromosomes),
		 b2g_bam_start(b1),
		 b2g_bam_mate_start(b1),
		 b2g_bam_end(b1),
		 b2g_bam_mate_end(b1));
}

int b2g_bam_qname_cmp(void* a, void* b) {
  return strnum_cmp(bam1_qname((bam1_t*)a), bam1_qname((bam1_t*)b));
}

int b2g_bams_highq(b2g_bam_t *b1, b2g_bam_t *b2, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  return !b2g_bams_overlap(b1, b2) && 
    (HIGH == b2g_bam_type(b1, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES)) &&
    (HIGH == b2g_bam_type(b2, MAPPING_QUALITY, MIN_ALIGNED_PCT, IGNORE_DUPLICATES));
}

// ACCESSORS

// Adapted from samtools/bam.c
int32_t b2g_bam_clippedlength(bam1_t *b) {
  const bam1_core_t c = b->core;
  const uint32_t *cigar = bam1_cigar(b);
  uint32_t k;
  int32_t l = 0;
  for (k = 0; k < c.n_cigar; ++k) {
    if ('S' == bam_cigar_opchr(bam_cigar_op(cigar[k]))) {
      l += bam_cigar_oplen(cigar[k]);
    }
  }
  return l;
}

char *b2g_bam_readgroup(bam1_t *bam) {
  return (char*)(bam_aux_get(bam, "RG") + 1);
}


b2g_bam_type_t b2g_bam_type(b2g_bam_t *b, int MAPPING_QUALITY, int MIN_ALIGNED_PCT, int IGNORE_DUPLICATES) {
  if (IGNORE_DUPLICATES && b2g_bam_dup(b)) return DUPLICATE;
  else if (b2g_bam_unmapped(b)) return UNMAPPED;
  else if (((double)(100 - MIN_ALIGNED_PCT)) * .01 * (double)b2g_bam_readlength(b) <= b2g_bam_clippedlength((bam1_t *)b)) return SPLIT;
  else if b2g_bam_lowq(b, MAPPING_QUALITY) return LOW;
  return HIGH;
}
