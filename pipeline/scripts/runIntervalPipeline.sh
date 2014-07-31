#!/bin/bash

set -e

INOUT_DIR=$1
CONFIG_FILE_PATH=$2

#Make the interval file, because output gets written to the same directory as the interval file.

cp PipelineSoftware/intervalCountingPipeline/intervals.txt $INOUT_DIR

echo "Running IntervalCounts"
pushd $INOUT_DIR
./../../../PipelineSoftware/intervalCountingPipeline/bin/IntervalCounts $CONFIG_FILE_PATH
popd