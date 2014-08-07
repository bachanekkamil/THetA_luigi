#include <assert.h>
#include "bam_files2bams.h"
#include "bam.h"
#include "b2g_bam.h"
#include "buffer.h"
#include "b2g_signal.h"
#include "sam.h"
#include "b2g_constants.h"
#include <time.h>
#include "b2g_chromosome.h"

// Consumes all split bam files and pushes their bams into an inter-process queue.

void bam_file2bams(samfile_t *bamfile, buffer_t* bams, b2g_chromosomes_t *chromosomes, int VERBOSE, unsigned long *count, time_t *prevt, unsigned int *num_skipped) {
  assert(bamfile);

  time_t currt;
  int sam_err = 0;
  // Copy each read into the bams buffer,
  while ((!MAX_BAMS_TO_READ || *count < MAX_BAMS_TO_READ) && -1 < (sam_err = samread(bamfile,(bam1_t*)buffer_buffer(bams)))) {
    if (sam_err < -1 || -1 == b2g_bam_chr(buffer_buffer(bams), chromosomes)) {
      (*num_skipped)++;
      continue;
    }
    assert(!b2g_signalp(buffer_buffer(bams)));

    // and advance the buffer.  
    buffer_flush(bams);
    if (VERBOSE && 0 == ++(*count) % BAMS_READ_PER_OUTPUT) {
      printf("%lu bams read (%d)\n", *count, (int)difftime((currt = time(NULL)), *prevt));
      *prevt = currt;
    }
  }
  b2g_bam_clear(buffer_buffer(bams)); // If we must overwrite a stale bam, free its data.
  b2g_signal_eof_write(buffer_buffer(bams)); // Write a B2G_SIGNAL_EOF after each file.
  buffer_flush(bams);  
}


void bam_files2bams(char *BAM_FILE, char *OUTPUT_PREFIX, int num_split, int LOW_MEMORY, buffer_t *bams, b2g_chromosomes_t *chromosomes, int VERBOSE, unsigned int *num_skipped) {
  assert(BAM_FILE && OUTPUT_PREFIX && num_split <= MAX_BAM_FILES && is_bool(LOW_MEMORY) && bams && is_bool(VERBOSE));

  int j;
  char filename[MAX_FILENAME_SIZE] = {};
  unsigned long count = 0;
  time_t prevt = time(NULL);

  // If we split the BAM up, 
  if (LOW_MEMORY) {
    // read from each sub-BAM in sequence.
    for (j=0; j<num_split; ++j) {
      sprintf(filename, "%s.%d.bam", OUTPUT_PREFIX, j); 
      samfile_t *bamfile = b2g_samfile_open(filename, "rb", 0);  
      bam_file2bams(bamfile, bams, chromosomes, VERBOSE, &count, &prevt, num_skipped);
      samclose(bamfile);    
      unlink(filename);
    }
  }
  // Otherwise, read directly once through.
  else {
    samfile_t *bamfile = b2g_samfile_open(BAM_FILE, "rb", 0);          
    bam_file2bams(bamfile, bams, chromosomes, VERBOSE, &count, &prevt, num_skipped);
    samclose(bamfile);    
  }
}

