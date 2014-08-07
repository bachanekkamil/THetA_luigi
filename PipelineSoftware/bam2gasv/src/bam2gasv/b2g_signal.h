// These signals represent out-of-band messages between components. They will be interspersed with bams and pairs in queues and buffers to signal that the receiving thread should either reset to process a new file or terminate to finish the program.

#ifndef B2G_SIGNAL_H
#define B2G_SIGNAL_H

// DATATYPES

typedef enum {
  B2G_SIGNAL_CONTINUE = -5, // Signals that no EOFs or EOAFs have been received during processing.
  B2G_SIGNAL_ERR = -4, // If this value reaches active code, there has been an error.
  B2G_SIGNAL_EOAF = -3, // End of all BAM files - terminate program.
  B2G_SIGNAL_EOF = -2 // End of a BAM file has been reached.
  // samtools uses -1 as a failure code, so -1 is "in-band" with respect to bam data
} b2g_signal_t;

// In case we need reserved space for a pointer to a signal value.
extern int B2G_SIGNAL_PTR_EOF;
extern int B2G_SIGNAL_PTR_EOAF;
extern int B2G_SIGNAL_PTR_ERR;

// PREDICATES

#define b2g_signalp(gasv_data) (b2g_signal_eof(gasv_data) || b2g_signal_eoaf(gasv_data) || b2g_signal_err(gasv_data))
#define b2g_signal_eof(gasv_data) (B2G_SIGNAL_EOF == *(int*)(gasv_data))
#define b2g_signal_eoaf(gasv_data) (B2G_SIGNAL_EOAF == *(int*)(gasv_data))
#define b2g_signal_err(gasv_data) (B2G_SIGNAL_ERR == *(int*)(gasv_data))


// MUTATORS

// Write an EOF to a pice of memory.
void b2g_signal_eof_write(void *);
// Write an EOAF to a pice of memory.
void b2g_signal_eoaf_write(void *);

#endif
