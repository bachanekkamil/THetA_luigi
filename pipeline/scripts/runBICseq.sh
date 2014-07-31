#!/bin/bash

#==============================================================================

set -e

# source $config

OUT_DIR=$1
TUMOR_CONC=$2
NORMAL_CONC=$3

#BICseq_INPUT="$baseDir/bicseq.input"

BICseq_INPUT=$4
PREFIX=$5

###
#	Create BIC-seq input
###
#if [ ! -f "$BICseq_INPUT.config" ]; then
# pushd "PipelineSoftware/BICseq/"
pushd $OUT_DIR
# echo $PWD
echo "Running createBICSeqInput..."
java -classpath ../../../PipelineSoftware/BICseq createBICSeqInput $TUMOR_CONC $NORMAL_CONC -OUTPUT_PREFIX $PREFIX
popd
#fi

BICseq_LOC="./PipelineSoftware/BICseq/PERL_pipeline/BICseq_1.1.2/BIC-seq"
BICseq_CONFIG=$PREFIX".config"
# BICseq_PREFIX=$5

###
#	Run BIC-seq pipeline
###
#if [ ! -d "$BICseqDIR" ]; then
pushd $OUT_DIR
echo "Running BIC-seq..."
../../../$BICseq_LOC/BIC-seq.pl $BICseq_CONFIG output $PREFIX
popd
#fi

