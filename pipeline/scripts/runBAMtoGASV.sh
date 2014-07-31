#!/bin/bash

set -e

INOUT_DIR=$1
BAM_FILE_PATH=$2
PREFIX=$3

echo "Running bam2gasv"
pushd $INOUT_DIR
./../../../../PipelineSoftware/bam2gasv/bin/bam2gasv $BAM_FILE_PATH -WRITE_CONCORDANT true -MAPPING_QUALITY 30 -OUTPUT_PREFIX $PREFIX
popd