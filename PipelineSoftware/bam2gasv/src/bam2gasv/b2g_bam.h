// Provides an interface to the samtools bam1_t structure.

#ifndef B2G_BAM_H
#define B2G_BAM_H

#include "bam.h"
#include "sam.h"
#include "list.h"
#include "b2g_chromosome.h"

// DATATYPES

typedef enum {
  UNMAPPED, DUPLICATE, SPLIT, LOW, HIGH
} b2g_bam_type_t;

typedef bam1_t b2g_bam_t;


// CONSTUCTORS


b2g_bam_t *b2g_bam_clone(b2g_bam_t*);
void b2g_bam_free(b2g_bam_t *);
void b2g_bam_clear(b2g_bam_t *);

// Normalize the orientations to ILLUMINA.
#define b2g_bam_lori_normalized(ori, PLATFORM) (MATEPAIR == PLATFORM ? !ori : ori)
#define b2g_bam_rori_normalized(ori, PLATFORM, first_is_first) ((MATEPAIR == PLATFORM || (SOLID == PLATFORM && first_is_first)) ? !ori : ori)

// PREDICATES

int b2g_bam_pair_split(b2g_bam_t *, b2g_bam_t *, int, int, int);
int b2g_cmp(int , int , int , int , int , int);
int b2g_bam_cmp(b2g_bam_t *, b2g_bam_t *, b2g_chromosomes_t *);
int b2g_bam_mate_cmp(b2g_bam_t *, b2g_chromosomes_t *);
int b2g_bam_qname_cmp(void* , void* );
int b2g_bams_highq(b2g_bam_t *, b2g_bam_t *, int, int, int);


// ACCESSORS

#define b2g_bam_qname(bam) (bam1_qname(((bam1_t*)(bam))))
#define b2g_bam_qname_length(bam) (((bam1_t*)(bam))->core.l_qname)
#define b2g_bam_chr(bam, chromosomes) (b2g_chromosomes_numbers_get(chromosomes, ((bam1_t*)(bam))->core.tid))
#define b2g_bam_mate_chr(bam, chromosomes) (b2g_chromosomes_numbers_get(chromosomes, ((bam1_t*)(bam))->core.mtid))
#define b2g_bam_ori(bam) (bam1_strand(((bam1_t*)(bam))))
#define b2g_bam_mate_ori(bam) (bam1_mstrand(((bam1_t*)(bam))))
#define b2g_bam_start(bam) (((bam1_t*)(bam))->core.pos + 1)
#define b2g_bam_mate_start(bam) (((bam1_t*)(bam))->core.mpos + 1)
#define b2g_bam_end(bam) (b2g_bam_start((bam1_t*)(bam)) + ((bam1_t*)(bam))->core.l_qseq - 1)
#define b2g_bam_mate_end(bam) (b2g_bam_mate_start((bam1_t*)(bam)) + ((bam1_t*)(bam))->core.l_qseq - 1)
#define b2g_bam_qual(bam) (((bam1_t*)(bam))->core.qual)
#define b2g_bam_unmapped(bam) ((bam)->core.flag & BAM_FUNMAP)
#define b2g_bam_readlength(bam) ((bam)->core.l_qseq)
#define b2g_bam_dup(bam) ((bam)->core.flag & BAM_FDUP)
#define b2g_bam_first_of_pair(bam) ((bam)->core.flag & BAM_FREAD1)
int32_t b2g_bam_clippedlength(bam1_t *);
char *b2g_bam_readgroup(bam1_t *);
b2g_bam_type_t b2g_bam_type(b2g_bam_t *, int , int , int );

#define b2g_bams_overlap(b1, b2) (pos_overlap(b2g_bam_start((b1)), b2g_bam_end((b1)), b2g_bam_start((b2)), b2g_bam_end((b2))))
// start1, end1, start2, end2
#define pos_overlap(s1, e1, s2, e2) (((s1) <= (s2) && (s2) <= (e1)) || ((s2) <= (s1) && (s1) <= (e2)))

#define b2g_bam_lowq(b, MAPPING_QUALITY) (b2g_bam_qual(b) < (MAPPING_QUALITY))
#define b2g_bams_lowq(b1, b2, MAPPING_QUALITY) (b2g_bam_lowq(b1, MAPPING_QUALITY) || b2g_bam_lowq(b2, MAPPING_QUALITY))

// SERIALIZATION
samfile_t* b2g_samfile_open(char*, char*, void*, ...);

#endif
