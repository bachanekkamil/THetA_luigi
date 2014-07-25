TFLAGS=-Wall -g
CFLAGS=-Wall -g
LDFLAGS=-lbam -lz -lpthread -lm


BAM2GASV_O=$(patsubst %.c,%.o,$(wildcard src/bam2gasv/*.c))

.PHONY: all clean test test/unit test/unit/btg_input2params test/integration test/integration/basic test/integration/lowmem test/integration/sd test/integration/pct test/integration/ambig test/integration/multilib test/integration/gasvpro test/integration/validate test/system test/system/basic test/performance test/performance/baseline test/performance/bam_files2bams test/performance/bam_files2unsorted_pairs test/performance/bam_files2pairs head

# By default, just compile the executable.
all: bin/bam2gasv

clean: 
	rm -rf bin
	rm -rf tmp

tmp: 
	mkdir -p tmp/bam_files2gasv_files
	mkdir -p tmp/bam_input2bam_files
	mkdir -p tmp/gasv_buffers2gasv_output
	mkdir -p tmp/bam2gasv/no_opt
	mkdir -p tmp/bam2gasv/sd
	mkdir -p tmp/bam2gasv/pct
	mkdir -p tmp/bam2gasv/all_opt
	mkdir -p tmp/bam2gasv/ambig
	mkdir -p tmp/disc_pair
	mkdir -p tmp/conc_pair
	mkdir -p tmp/sorted_pairs2gasv_files
	mkdir -p tmp/gasv_pairs2lowq_file
	cp test/bam_files2gasv_files/* tmp/bam_files2gasv_files

# Compile the samtools library if needed
lib/samtools/libbam.a: 
	cd lib/samtools && $(MAKE)

# Download Example.bam if needed
test/Example.bam:
	wget http://gasv.googlecode.com/files/Example.bam -P test/

bin: 
	mkdir -p bin

# Compile the executable
bin/bam2gasv: src/bam2gasv/*.c src/bam2gasv/*.h lib/func/src/*.c lib/func/src/*.h  lib/samtools/libbam.a
	mkdir -p bin
	$(CC) $(filter %.c, $^) -o bin/bam2gasv -Ilib/samtools -Ilib/func/src -Llib/samtools $(CFLAGS) $(LDFLAGS)



# TESTS

TEST_DEPENDENCIES=src/bam2gasv/*.c src/bam2gasv/*.h lib/func/src/*.c lib/func/src/*.h  lib/samtools/libbam.a test/test_utils.c test/test_utils.h tmp/bin test/Example.bam tmp/count

# Folders for holding test executables
# System tests rely on the main executable and do not have special test binaries.
tmp/bin:
	mkdir -p tmp/integration/bin
	mkdir -p tmp/unit/bin
	mkdir -p tmp/performance/bin

# Utility for extracting the first n reads from a bam file.
bin/head: util/head.c lib/samtools/libbam.a bin
	$(CC) util/head.c -o bin/head -Ilib/samtools -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)
head: bin/head

# Test utility for counting the reads in a BAM file.
tmp/count: test/count.c lib/samtools/libbam.a tmp/bin
	mkdir -p tmp/integration/basic
	$(CC) test/count.c -o tmp/count -Ilib/samtools -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)


# Run all tests
test: test/unit test/integration test/system 

# SYSTEM

# run all system tests
test/system: test/system/basic

# test basic system
tmp/system/basic: 
	mkdir -p tmp/system/basic

test/system/basic: bin/bam2gasv tmp/system/basic
	@echo "$(shell ./bin/bam2gasv test/Example.bam -OUTPUT_PREFIX tmp/system/basic/Example -CUTOFF_LMINLMAX EXACT=153,247 -VERBOSE false -GASVPRO true 2>&1)"
	@echo "$(shell diff -q test/integration/basic/Example_all.divergent tmp/system/basic/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/basic/Example_all.insertion tmp/system/basic/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.inversion tmp/system/basic/Example_all.inversion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.translocation tmp/system/basic/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/basic/Example_all.deletion tmp/system/basic/Example_all.deletion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.concordant tmp/system/basic/Example_all.concordant)"
	@echo "$(shell diff -q test/integration/basic/Example.info tmp/system/basic/Example.info)"
	@echo "$(shell diff -q test/system/basic/Example.gasv.in tmp/system/basic/Example.gasv.in)"
	@echo "$(shell diff -q test/system/basic/Example.gasvpro.in tmp/system/basic/Example.gasvpro.in)"


# INTEGRATION

# run all integration level tests
test/integration: test/integration/basic test/integration/sd test/integration/pct test/integration/ambig test/integration/lowmem test/integration/multilib test/integration/gasvpro test/integration/validate

# test basic integration
tmp/integration/bin/basic: test/integration/basic.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/basic
	rm -f tmp/integration/basic/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/basic -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/basic: tmp/integration/bin/basic
	@echo "$(shell ./tmp/integration/bin/basic 2>&1)"
	@echo "$(shell diff -q test/integration/basic/Example_all.divergent tmp/integration/basic/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/basic/Example_all.insertion tmp/integration/basic/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.inversion tmp/integration/basic/Example_all.inversion)"
#	@echo "$(shell diff -q test/integration/basic/Example_all.translocation tmp/integration/basic/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/basic/Example_all.deletion tmp/integration/basic/Example_all.deletion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.concordant tmp/integration/basic/Example_all.concordant)"
	@echo "$(shell diff -q test/integration/basic/Example.info tmp/integration/basic/Example.info)"
	@echo "$(shell diff -q test/integration/basic/Example.gasv.in tmp/integration/basic/Example.gasv.in)"
	@echo "$(shell diff -q test/integration/basic/Example.gasvpro.in tmp/integration/basic/Example.gasvpro.in)"


# test standard deviation computation
tmp/integration/bin/sd: test/integration/sd.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/sd
	rm -f tmp/integration/sd/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/sd -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/sd: tmp/integration/bin/sd
	@echo "$(shell ./tmp/integration/bin/sd 2>&1)"
	@echo "$(shell diff -q test/integration/sd/Example_all.divergent tmp/integration/sd/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/sd/Example_all.insertion tmp/integration/sd/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/sd/Example_all.inversion tmp/integration/sd/Example_all.inversion)"
	@echo "$(shell diff -q test/integration/sd/Example_all.translocation tmp/integration/sd/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/sd/Example_all.deletion tmp/integration/sd/Example_all.deletion)"
	@echo "$(shell diff -q test/integration/sd/Example_all.concordant tmp/integration/sd/Example_all.concordant)"
	@echo "$(shell diff -q test/integration/sd/Example.info tmp/integration/sd/Example.info)"
	@echo "$(shell diff -q test/integration/sd/Example.gasv.in tmp/integration/sd/Example.gasv.in)"
	@echo "$(shell diff -q test/integration/sd/Example.gasvpro.in tmp/integration/sd/Example.gasvpro.in)"
#	@echo "$(shell ./tmp/count tmp/integration/sd/Example_lowqual.bam 113008)"

# test percent quantile computation
tmp/integration/bin/pct: test/integration/pct.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/pct
	rm -f tmp/integration/pct/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/pct -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/pct: tmp/integration/bin/pct
	@echo "$(shell ./tmp/integration/bin/pct 2>&1)"
	@echo "$(shell diff -q test/integration/basic/Example_all.divergent tmp/integration/pct/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/pct/Example_all.insertion tmp/integration/pct/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.inversion tmp/integration/pct/Example_all.inversion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.translocation tmp/integration/pct/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/pct/Example_all.deletion tmp/integration/pct/Example_all.deletion)"
	@echo "$(shell diff -q test/integration/pct/Example_all.concordant tmp/integration/pct/Example_all.concordant)"
	@echo "$(shell diff -q test/integration/pct/Example.info tmp/integration/pct/Example.info)"
	@echo "$(shell diff -q test/integration/pct/Example.gasv.in tmp/integration/pct/Example.gasv.in)"
	@echo "$(shell diff -q test/integration/pct/Example.gasvpro.in tmp/integration/pct/Example.gasvpro.in)"
#	@echo "$(shell ./tmp/count tmp/integration/pct/Example_lowqual.bam 107032)"
#	@echo "$(shell ./tmp/count tmp/integration/pct/Example_splitread.bam 11434)"


# test ambig

tmp/integration/bin/ambig: test/integration/ambig.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/ambig
	rm -f tmp/integration/ambig/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/ambig -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/ambig: tmp/integration/bin/ambig
	@echo "$(shell ./tmp/integration/bin/ambig 2>&1)"
	@echo "$(shell diff -q test/integration/ambig/Example_all.divergent tmp/integration/ambig/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/ambig/Example_all.insertion tmp/integration/ambig/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/ambig/Example_all.inversion tmp/integration/ambig/Example_all.inversion)"
	@echo "$(shell diff -q test/integration/ambig/Example_all.translocation tmp/integration/ambig/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/ambig/Example_all.deletion tmp/integration/ambig/Example_all.deletion)"


# test low mem integration
tmp/integration/bin/lowmem: test/integration/lowmem.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/lowmem
	rm -f tmp/integration/lowmem/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/lowmem -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/lowmem: tmp/integration/bin/lowmem
	@echo "$(shell ./tmp/integration/bin/lowmem 2>&1)"
	@echo "$(shell diff -q test/integration/basic/Example_all.divergent tmp/integration/lowmem/Example_all.divergent)"
	@echo "$(shell diff -q test/integration/basic/Example_all.insertion tmp/integration/lowmem/Example_all.insertion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.inversion tmp/integration/lowmem/Example_all.inversion)"
	@echo "$(shell diff -q test/integration/basic/Example_all.translocation tmp/integration/lowmem/Example_all.translocation)"
	@echo "$(shell diff -q test/integration/basic/Example_all.deletion tmp/integration/lowmem/Example_all.deletion)"
	@echo "$(shell diff -q test/integration/lowmem/Example.info tmp/integration/lowmem/Example.info)"
	@echo "$(shell diff -q test/integration/lowmem/Example.gasv.in tmp/integration/lowmem/Example.gasv.in)"
	@echo "$(shell diff -q test/integration/lowmem/Example.gasvpro.in tmp/integration/lowmem/Example.gasvpro.in)"

# test multiple library integration
tmp/integration/bin/multilib: test/integration/multilib.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/multilib
	rm -f tmp/integration/multilib/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/multilib -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/multilib: tmp/integration/bin/multilib
	@echo "$(shell ./tmp/integration/bin/multilib 2>&1)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.divergent tmp/integration/multilib/Example2_Solexa-12036.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.insertion tmp/integration/multilib/Example2_Solexa-12036.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.inversion tmp/integration/multilib/Example2_Solexa-12036.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.translocation tmp/integration/multilib/Example2_Solexa-12036.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.deletion tmp/integration/multilib/Example2_Solexa-12036.deletion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.concordant tmp/integration/multilib/Example2_Solexa-12036.concordant)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.divergent tmp/integration/multilib/Example2_Solexa-12037.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.insertion tmp/integration/multilib/Example2_Solexa-12037.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.inversion tmp/integration/multilib/Example2_Solexa-12037.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.translocation tmp/integration/multilib/Example2_Solexa-12037.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.deletion tmp/integration/multilib/Example2_Solexa-12037.deletion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.concordant tmp/integration/multilib/Example2_Solexa-12037.concordant)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.divergent tmp/integration/multilib/Example2_Solexa-13877.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.insertion tmp/integration/multilib/Example2_Solexa-13877.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.inversion tmp/integration/multilib/Example2_Solexa-13877.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.translocation tmp/integration/multilib/Example2_Solexa-13877.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.deletion tmp/integration/multilib/Example2_Solexa-13877.deletion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.concordant tmp/integration/multilib/Example2_Solexa-13877.concordant)"

	@echo "$(shell diff -q test/integration/multilib/Example2.info tmp/integration/multilib/Example2.info)"
	@echo "$(shell diff -q test/integration/multilib/Example2.gasv.in tmp/integration/multilib/Example2.gasv.in)"

# test gasvpro concordant merging integration 
tmp/integration/bin/gasvpro: test/integration/gasvpro.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/gasvpro
	rm -f tmp/integration/gasvpro/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/gasvpro -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/gasvpro: tmp/integration/bin/gasvpro
	@echo "$(shell ./tmp/integration/bin/gasvpro 2>&1)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.divergent tmp/integration/gasvpro/Example2_Solexa-12036.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.insertion tmp/integration/gasvpro/Example2_Solexa-12036.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.inversion tmp/integration/gasvpro/Example2_Solexa-12036.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.translocation tmp/integration/gasvpro/Example2_Solexa-12036.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.deletion tmp/integration/gasvpro/Example2_Solexa-12036.deletion)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.divergent tmp/integration/gasvpro/Example2_Solexa-12037.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.insertion tmp/integration/gasvpro/Example2_Solexa-12037.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.inversion tmp/integration/gasvpro/Example2_Solexa-12037.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.translocation tmp/integration/gasvpro/Example2_Solexa-12037.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.deletion tmp/integration/gasvpro/Example2_Solexa-12037.deletion)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.divergent tmp/integration/gasvpro/Example2_Solexa-13877.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.insertion tmp/integration/gasvpro/Example2_Solexa-13877.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.inversion tmp/integration/gasvpro/Example2_Solexa-13877.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.translocation tmp/integration/gasvpro/Example2_Solexa-13877.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.deletion tmp/integration/gasvpro/Example2_Solexa-13877.deletion)"

	@echo "$(shell diff -q test/integration/gasvpro/Example2_all.concordant tmp/integration/gasvpro/Example2_all.concordant)"
	@echo "$(shell diff -q test/integration/multilib/Example2.info tmp/integration/gasvpro/Example2.info)"
	@echo "$(shell diff -q test/integration/gasvpro/Example2.gasv.in tmp/integration/gasvpro/Example2.gasv.in)"

# test gasvpro concordant merging with low memory integration 
tmp/integration/bin/lowpro: test/integration/lowpro.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/lowpro
	rm -f tmp/integration/lowpro/*
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/lowpro -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/lowpro: tmp/integration/bin/lowpro
	@echo "$(shell ./tmp/integration/bin/lowpro 2>&1)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.divergent tmp/integration/lowpro/Example2_Solexa-12036.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.insertion tmp/integration/lowpro/Example2_Solexa-12036.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.inversion tmp/integration/lowpro/Example2_Solexa-12036.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.translocation tmp/integration/lowpro/Example2_Solexa-12036.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12036.deletion tmp/integration/lowpro/Example2_Solexa-12036.deletion)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.divergent tmp/integration/lowpro/Example2_Solexa-12037.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.insertion tmp/integration/lowpro/Example2_Solexa-12037.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.inversion tmp/integration/lowpro/Example2_Solexa-12037.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.translocation tmp/integration/lowpro/Example2_Solexa-12037.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-12037.deletion tmp/integration/lowpro/Example2_Solexa-12037.deletion)"

	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.divergent tmp/integration/lowpro/Example2_Solexa-13877.divergent)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.insertion tmp/integration/lowpro/Example2_Solexa-13877.insertion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.inversion tmp/integration/lowpro/Example2_Solexa-13877.inversion)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.translocation tmp/integration/lowpro/Example2_Solexa-13877.translocation)"
	@echo "$(shell diff -q test/integration/multilib/Example2_Solexa-13877.deletion tmp/integration/lowpro/Example2_Solexa-13877.deletion)"

	@echo "$(shell diff -q test/integration/gasvpro/Example2_all.concordant tmp/integration/lowpro/Example2_all.concordant)"
	@echo "$(shell diff -q test/integration/lowpro/Example2.info tmp/integration/lowpro/Example2.info)"
	@echo "$(shell diff -q test/integration/lowpro/Example2.gasv.in tmp/integration/lowpro/Example2.gasv.in)"

# test validate integration
tmp/integration/bin/validate: test/integration/validate.c $(TEST_DEPENDENCIES)
	mkdir -p tmp/integration/validate
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/integration/bin/validate -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/integration/validate: tmp/integration/bin/validate
	@echo "$(shell ./tmp/integration/bin/validate test/tcga.sorted.bam)"	


# UNIT 

# run all unit tests
test/unit: test/unit/btg_input2params

tmp/unit/bin/btg_input2params: test/unit/btg_input2params.c $(TEST_DEPENDENCIES)
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/unit/bin/btg_input2params -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/unit/btg_input2params: tmp/unit/bin/btg_input2params
	@echo "$(shell ./tmp/unit/bin/btg_input2params 2>&1)"



# PERFORMANCE 

# compile all performance tests
tmp/performance: tmp/performance/bin/baseline tmp/performance/bin/bam_files2bams tmp/performance/bin/bam_files2unsorted_pairs tmp/performance/bin/bam_files2pairs

tmp/performance/bin/baseline: test/performance/baseline.c $(TEST_DEPENDENCIES)
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/performance/bin/baseline -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/performance/baseline: tmp/performance/bin/baseline



tmp/performance/bin/bam_files2bams: test/performance/bam_files2bams.c $(TEST_DEPENDENCIES)
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/performance/bin/bam_files2bams -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/performance/bam_files2bams: tmp/performance/bin/bam_files2bams



tmp/performance/bin/bam_files2unsorted_pairs: test/performance/bam_files2unsorted_pairs.c $(TEST_DEPENDENCIES)
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/performance/bin/bam_files2unsorted_pairs -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/performance/bam_files2unsorted_pairs: tmp/performance/bin/bam_files2unsorted_pairs


tmp/performance/bin/bam_files2pairs: test/performance/bam_files2pairs.c $(TEST_DEPENDENCIES)
	$(CC) $(filter-out src/bam2gasv/bam2gasv.c, $(filter %.c, $^)) -o tmp/performance/bin/bam_files2pairs -Ilib/samtools -Isrc/bam2gasv -Ilib/func/src -Llib/samtools $(CFLAGS) $(TFLAGS) $(LDFLAGS)

test/performance/bam_files2pairs: tmp/performance/bin/bam_files2pairs

