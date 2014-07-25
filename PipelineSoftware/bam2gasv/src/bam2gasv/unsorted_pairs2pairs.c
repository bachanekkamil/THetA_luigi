#include <assert.h>
#include <pthread.h>
#include "unsorted_pairs2pairs.h"
#include "b2g_pair.h"
#include "buffer.h"
#include "b2g_signal.h"
#include "b2g_variant_type.h"
#include <sched.h>

static void _write_eofs(list_t **lib2unsorted_pairs, list_t lib2sorted_pairs[], buffer_t *pairs, int NUM_LIBS, int vtype, int merging_concs, int DEBUG_LEVEL) {
  int i;
  for (i=0; i<NUM_LIBS; i++) {
    b2g_pair_sort((*(lib2unsorted_pairs + i))->cdr, &lib2sorted_pairs[merging_concs ? 0 : i]);    
  } 
  for (i=0; i<NUM_LIBS; i++) {
    if (!merging_concs || !i) buffer_write_unlock(pairs, &lib2sorted_pairs[i]);
    bzero(&lib2sorted_pairs[i], sizeof(list_t));
  }
}

void unsorted_pairs2pairs(list_t **file2lib2unsorted_pairs, buffer_t *pairs, int NUM_LIBS, b2g_variant_t VTYPE, int GASV_PRO, int num_files, int DEBUG_LEVEL) {

  int i, j, merging_concs = CONC == VTYPE && GASV_PRO;
  list_t lib2sorted_pairs[NUM_LIBS], **lib2unsorted_pairs;
  list_t *tmp;
  //TODOint counts[3] = {};
  bzero(lib2sorted_pairs, sizeof(list_t) * NUM_LIBS);

  for (i=0; i<num_files; i++) {
    lib2unsorted_pairs = file2lib2unsorted_pairs + (i * NUM_LIBS);
    // Until one of the list buffers has an EOF signal in its car,
    while (!b2g_signal_eof(&(*lib2unsorted_pairs)->car)) { 
      // sort the available pairs for each library.
      for (j=0; j<NUM_LIBS; j++) {
	tmp = list_buffer_read_try(*(lib2unsorted_pairs + j)); //TODO
	//TODOif (merging_concs && tmp) counts[j] += list_length(tmp);
//printf("%d from lib %d\n", list_length(tmp), j);
	b2g_pair_sort(tmp, &lib2sorted_pairs[merging_concs ? 0 : j]);
      }
      sched_yield();
    }
    //TODOprintf("%d %d %d\n", counts[0], counts[1], counts[2]);
    _write_eofs(lib2unsorted_pairs, lib2sorted_pairs, pairs, NUM_LIBS, VTYPE, merging_concs, DEBUG_LEVEL);
  }
}

void* unsorted_pairs2pairs_start(void *args) {
  int *NUM_LIBS, *VTYPE, *GASV_PRO, *num_files, *DEBUG_LEVEL;
  list_t **file2lib2unsorted_pairs;
  buffer_t *pairs;

  list_values(args, &file2lib2unsorted_pairs, &pairs, &NUM_LIBS, &VTYPE, &GASV_PRO, &num_files, &DEBUG_LEVEL);

  unsorted_pairs2pairs(file2lib2unsorted_pairs, pairs, *NUM_LIBS, *VTYPE, *GASV_PRO, *num_files, *DEBUG_LEVEL);

  list_free(args, NULL);
  return NULL;
}

void unsorted_pairs2pairs_init(pthread_t b2g_pair2sorted_pairs_threads[NUM_VTYPES],int *NUM_LIBS, list_t **vtype2file2lib2unsorted_pairs, buffer_t *vtype2pairs[NUM_VTYPES], int *DEBUG_LEVEL, int WRITE_CONCORDANT, int *GASV_PRO, int *num_files) {
  assert(b2g_pair2sorted_pairs_threads && NUM_LIBS && is_positive(*NUM_LIBS));
  int i;

  // Spawn a separate thread for each vtype.
  for (i=0; i<NUM_VTYPES; i++) {
    if (!WRITE_CONCORDANT && CONC == i) continue;
    pthread_create(&b2g_pair2sorted_pairs_threads[i], NULL, unsorted_pairs2pairs_start, list(7, vtype2file2lib2unsorted_pairs + (i * *num_files * *NUM_LIBS),  vtype2pairs[i], NUM_LIBS, &VARIANT_TYPES[i], GASV_PRO, num_files, DEBUG_LEVEL));
  }
}
