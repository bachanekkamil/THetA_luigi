#include <assert.h> 
#include "b2g_signal.h"

// DATATYPES

int B2G_SIGNAL_PTR_EOF = B2G_SIGNAL_EOF;
int B2G_SIGNAL_PTR_EOAF = B2G_SIGNAL_EOAF;
int B2G_SIGNAL_PTR_ERR = B2G_SIGNAL_ERR;

// MUTATORS

void b2g_signal_eof_write(void *channel) {
  assert(channel);
  assert(0 == *((int *)channel));
  *((int *)channel) = B2G_SIGNAL_EOF;
}


void b2g_signal_eoaf_write(void *channel) {
  assert(channel);
  assert(0 == *((int *)channel));
  *((int *)channel) = B2G_SIGNAL_EOAF;
}
