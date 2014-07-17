#!/bin/bash
source $config
#Run preprocessing for PREGO on EGA data.
#Run PREGO

CYTO_BANDS=$1
OUTPUT_DIR=$2
TUMOR_DIR=$3
NORMAL_DIR=$4
MIN=$5
PREFIX=$6

TUMOR_INPUT=$B2GV_outputT
NORMAL_INPUT=$B2GV_outputN

INTERVAL_DIR="./software/intervalCountingPipeline" 
###############################################################################
#STEP 1: Run Filtering -- ASSUMES THE CORRECT FILES HAVE BEEN CREATED
TUMOR_CLUSTERS=$gasvOutputTumor
NORMAL_NOCLUSTERS=$gasvOutputNormal
# Create prego directory
PREGO=$pregoDir

if [ ! -d $PREGO ]; then
	echo "Making directory prego"
	mkdir $PREGO
fi
MIN_DIR=$PREGO"/min"$MIN
PRUNED=$MIN_DIR/"$PREFIX.gasv.in.clusters.min"$MIN".final"
if [ ! -d $MIN_DIR ]; then
	mkdir $MIN_DIR
fi

if [ ! -f $PRUNED ]; then 
	java -jar $INTERVAL_DIR/bin/PruneClusters.jar $TUMOR_CLUSTERS $NORMAL_NOCLUSTERS \
		--cytoBandsFile $CYTO_BANDS --minClusterSize $MIN --PREGO --verbose \
		--removeXY > $TUMOR_CLUSTERS.out 
	mv $TUMOR_CLUSTERS.final $PRUNED
fi

###############################################################################
#STEP 2: Build Intervals
EXCLUDE="/data/compbio/publicdata/EGA_Data/EGAD00001000138/exclude.txt"
TUMOR_CONC="$TUMOR_DIR/TUMOR.concordant"
NORMAL_CONC="$NORMAL_DIR/NORMAL.concordant"

if [ ! -f $TUMOR_DIR"/TUMOR.concordant" ]; then
	cat $TUMOR_DIR/*.concordant > $TUMOR_DIR/TUMOR.concordant
fi
if [ ! -f $NORMAL_DIR"/NORMAL.concordant" ]; then
	cat $NORMAL_DIR/*.concordant > $NORMAL_DIR/NORMAL.concordant
fi

if [ ! -f "$PRUNED.intervals" ]; then
	java -jar $INTERVAL_DIR"/bin/buildIntervalsPREGO.jar" $PRUNED $CYTO_BANDS\
		-TUMOR_CONCORDANT $TUMOR_CONC -NORMAL_CONCORDANT $NORMAL_CONC\
		-EXCLUDE_FILE $EXCLUDE
fi
###############################################################################
#STEP 3: Concordant coverage
INTERVAL_FILE=$PRUNED".intervals"
VARIANT_FILE=$PRUNED".variants"
PARAM_FILE=$PRUNED".parameters"

if [ ! -f "${INTERVAL_FILE}_processed" ]; then
	pushd $INTERVAL_DIR"/bin"
	./IntervalCounts $PARAM_FILE
	popd
fi
###############################################################################
#STEP 4: Run Prego
FINAL_FILE=$INTERVAL_FILE"_processed"
export ILOG_LICENSE_FILE=/local/projects/cplex/ilm/site.access.ilm

if [ ! -f $FINAL_FILE".normal.withTelo" ]; then
	./software/PREGO/bin/runPREGO $FINAL_FILE $VARIANT_FILE -HANDLE_TELOS True\
		  -OUTPUT_PREFIX ${FINAL_FILE}".normal.withTelo"
fi
