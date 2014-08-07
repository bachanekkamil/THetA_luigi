#include <time.h>
#include "sam.h"
#include "bam.h"
#include "list.h"
#include "hash.h"
#include "b2g_constants.h"
#include "b2g_bam.h"
#include "b2g_chromosome.h"


//Creates empty bucket files for bucketing the reads by qname.
static int _make_split_files(bam1_t* bamrecord, samfile_t* bamfile, samfile_t*** hashfiles, char *OUTPUT_PREFIX) {
  int num_files = 0;
  list_t* hashfile_list = NULL;  
  samfile_t* hashfile;
  while (num_files < MAX_BAM_FILES && (hashfile = b2g_samfile_open("%s.%d.bam", "wb", bamfile->header, OUTPUT_PREFIX, num_files))) {
    hashfile_list = cons(hashfile, hashfile_list);
    ++num_files;
  }
  *hashfiles = (samfile_t**)list2array(hashfile_list);
  list_free(hashfile_list, NULL);
  return num_files;
}

//Read through bamfile and split its reads up into the hashfiles "buckets" based on hash of the qname.
static void _fill_split_files(samfile_t* bamfile, bam1_t* bamrecord, b2g_chromosomes_t *chromosomes, int num_files, samfile_t** hashfiles, int WRITE_LOWQ, int WRITE_SPLITREAD, int MAPPING_QUALITY, int VERBOSE) {
  assert(num_files && chromosomes);
  time_t currt, prevt = time(NULL);
  unsigned long count = 0;
  while (-1 < samread(bamfile, bamrecord)) {
    //TODO check pairing duplicate stuff
    if ((!WRITE_LOWQ && !WRITE_SPLITREAD) && bamrecord->core.qual < MAPPING_QUALITY) continue;
    int file_i = djb2_hash(bam1_qname(bamrecord)) % num_files;
    samwrite(hashfiles[file_i], bamrecord);
    if (VERBOSE && 0 == ++count % BAMS_READ_PER_OUTPUT) {
      printf("%lu bams written (%d)\n", count, (int)difftime((currt = time(NULL)), prevt));
      prevt = currt;
    }
  }
}

//Close files and free resources.
static void _close_split_files(samfile_t* bamfile, bam1_t* bamrecord, int num_files, samfile_t** hashfiles) {
  int i;
  for (i=0; i<num_files; ++i) samclose(hashfiles[i]);
  free(hashfiles);
  samclose(bamfile);
  bam_destroy1(bamrecord);
}

//Fragments the file at bam_path into num_files different temporary files (reads are bucketed into temp files by hash of qname).
int params2bam_files(char *BAM_PATH, char *OUTPUT_PREFIX, int WRITE_LOWQ, int WRITE_SPLITREAD, int MAPPING_QUALITY, int VERBOSE, b2g_chromosomes_t *chromosomes) {
  int num_files;
  samfile_t** hashfiles;
  bam1_t* bamrecord = bam_init1();
  samfile_t* bamfile = samopen(BAM_PATH, "rb", 0);
  if (!bamfile) return -1;

  //Create a bam file for each "alphabetical" (by hash) range of qnames in the original file,
  num_files = _make_split_files(bamrecord, bamfile, &hashfiles, OUTPUT_PREFIX);
  //and sort all bamrecords from the original file into the subfiles based on a hash of the qname.  
  _fill_split_files(bamfile, bamrecord, chromosomes, num_files, hashfiles, WRITE_LOWQ, WRITE_SPLITREAD, MAPPING_QUALITY, VERBOSE);  

  //Finally, close up all the files.
  _close_split_files(bamfile, bamrecord, num_files, hashfiles);
  return num_files;
}


