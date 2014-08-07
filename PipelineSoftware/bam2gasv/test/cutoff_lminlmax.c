#include <string.h>
#include <assert.h>
#include "cutoff_lminlmax.h"
#include "gasv_library_header.h"
#include "list.h"

void check_cutoffs_from_file(library_cutoff_t lib_config[]) {
  assert(SD == lib_config[0].mode);
  assert(10 == lib_config[0].X);
  
  assert(PCT == lib_config[1].mode);
  assert(20 == lib_config[1].X);

  assert(EXACT == lib_config[2].mode);
  assert(30 == lib_config[2].X);
  assert(40 == lib_config[2].Y);
}

void test_parse_cutoff_mode() {

  int X, Y;
  char name[256];

  // SD
  assert(SD == parse_cutoff_mode("SD=10", &X, &Y, name));
  assert(10 == X);

  // PCT
  assert(PCT == parse_cutoff_mode("PCT=20", &X, &Y, name));
  assert(20 == X);

  // EXACT
  assert(EXACT == parse_cutoff_mode("EXACT=30,40", &X, &Y, name));
  assert(30 == X);
  assert(40 == Y);
 
  // FILENAME
  assert(FILENAME == parse_cutoff_mode("FILE=cutoff_file", &X, &Y, name));
  assert(!strcmp("cutoff_file", name));

}
 
/*
void test_cutoff_file_parse_line() {

  int X, Y;
  char name[256];

  // SD
  assert(SD == cutoff_file_parse_line("libname1 SD=10", &X, &Y, name));
  assert(10 == X);
  assert(!strcmp(name, "libname1"));

  // PCT
  assert(PCT == cutoff_file_parse_line("libname2 PCT=20%", &X, &Y, name));
  assert(20 == X);
  assert(!strcmp(name, "libname2"));

  // EXACT
  assert(EXACT == cutoff_file_parse_line("libname3 EXACT=30,40", &X, &Y, name));
  assert(30 == X);
  assert(40 == Y);
  assert(!strcmp(name, "libname3"));
}
*/

void test_cutoff_file_parse() {
  int num_libs = 3;
  library_cutoff_t lib_config[num_libs];
  gasv_library_header_t glib = {list(num_libs, "libname1", "libname2", "libname3"), NULL, 1};

  bzero(lib_config, num_libs * sizeof(library_cutoff_t));
  assert(0 == cutoff_file_parse(lib_config, "test/cutoff_lminlmax/cutoff_lminlmax.test", &glib));
  
  check_cutoffs_from_file(lib_config);
  list_free(glib.libs, NULL);
}


void test_cutoffs_init() {
  library_cutoff_t cutoff_configs[3];
  bzero(cutoff_configs, 3 * sizeof(library_cutoff_t));
  library_cutoff_t cutoff_template = {PCT, 10, -1, NULL};
  gasv_library_header_t glib = {list(3, "libname1", "libname2", "libname3"), NULL, 1};
  assert(0 == cutoffs_init(cutoff_configs, &cutoff_template, &glib));

  assert(PCT == cutoff_configs[1].mode);
  assert(10 == cutoff_configs[2].X);

  cutoff_template.mode = FILENAME;
  cutoff_template.name = "test/cutoff_lminlmax/cutoff_lminlmax.test";

  bzero(cutoff_configs, 3 * sizeof(library_cutoff_t));
  assert(0 == cutoffs_init(cutoff_configs, &cutoff_template, &glib));
  
  check_cutoffs_from_file(cutoff_configs);

  list_free(glib.libs, NULL);
}

void test_bam_path2gasv_library_header() {
  int LIB_SEP = 0;

  library_cutoff_t exact_cutoff = {EXACT, 53, 167, NULL};
  library_cutoff_t sd_cutoff = {SD, 1, -1, NULL};
  library_cutoff_t pct_cutoff = {PCT, 75, -1, NULL};
  
  gasv_library_header_t *glib = bam_path2gasv_library_header("test/cutoff_lminlmax/NA18507_7_nolibs.bam", LIB_SEP, &exact_cutoff);
  assert(53 == glib->lib2min_max[0][0]);
  assert(167 == glib->lib2min_max[0][1]);
  gasv_library_header_free(glib);
  
  glib = bam_path2gasv_library_header("/home/apocalypsemystic/data/gasv/NA18507_100000.bam", LIB_SEP, &sd_cutoff);
  printf("MIN: %d MAX: %d\n", glib->lib2min_max[0][0], glib->lib2min_max[0][1]);
  //printf("BLAH: %d\n", glib->lib2min_max[0][0]);
  gasv_library_header_free(glib);


  /*
  glib = bam_path2gasv_library("/home/apocalypsemystic/data/gasv/NA18507_100000.bam", LIB_SEP, &pct_cutoff);
  printf("MIN: %d MAX: %d\n", glib->lib2min_max[0][0], glib->lib2min_max[0][1]);
  gasv_library_header_free(glib);
  */
  
}

void test_sort_lengths() {
  int i;
  int l1[] = {4, 2, 3, 0, 1};
  sort_lengths(l1, 5);
  for (i=0; i<5; i++) {
    assert(i == l1[i]);
  }
}

int main() {

  test_parse_cutoff_mode();
  //test_cutoff_file_parse_line();
  test_cutoff_file_parse();
  test_cutoffs_init();
  test_bam_path2gasv_library_header();
  test_sort_lengths();

  return 0;
}
