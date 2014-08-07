#include <stdlib.h>
#include "b2g_constants.h"

char  ORIENTATION_CHARS[] = {'+', '-'};
char* BOOLS[] = {"false", "true"};
char* PLATFORMS[] = {"Illumina", "SOLiD", "MatePair"};


int B2G_HASH_SIZE = 4500; // Used as a scaling factor with BAM file size
int MAX_BAM_FILES = 50;
int BAMS_READ_PER_OUTPUT = 10000000;
//int BAMS_READ_PER_OUTPUT = 500000;
