#!/bin/bash

set -e

INOUT_DIR=$1
BAM_FILE_PATH=$2
PREFIX=$3
NORMAL=$4

echo "Running bam2gasv"
pushd $INOUT_DIR
./../../../../PipelineSoftware/bam2gasv/bin/bam2gasv $BAM_FILE_PATH -WRITE_CONCORDANT true -MAPPING_QUALITY 30 -OUTPUT_PREFIX $NORMAL
#Rename files so that other scripts know where the concordant files are.


if [ $NORMAL == "NORMAL" ]; then
	for i in NORMAL*lib*
	do
	   mv $i `echo $i | sed -e 's/NORMAL_.*-lib/NORMAL_'$PREFIX'-lib/'`
	done
else
	for i in TUMOR*lib*
	do
	   mv $i `echo $i | sed -e 's/TUMOR_.*-lib/TUMOR_'$PREFIX'-lib/'`
	done
fi
popd
