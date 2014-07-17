#!/bin/bash

#==============================================================================
#title :	runBICseq.sh
#author:	Gryte Satas
#date:	2013-09-18
#description:	
#usage:	runBICseq.sh tumorConcordantFile normalConcordantFile
#==============================================================================

set -e

source $config

TUMOR_CONC=$1
NORMAL_CONC=$2

BICseq_INPUT="$baseDir/bicseq.input"

###
#	Create BIC-seq input
###
if [ ! -f "$BICseq_INPUT.config" ]; then
	pushd "$software/BICseq/"
	echo "Running createBICSeqInput..."
	java createBICSeqInput $TUMOR_CONC $NORMAL_CONC -OUTPUT_PREFIX $BICseq_INPUT
	popd
fi

BICseq_LOC="./software/BICseq/PERL_pipeline/BICseq_1.1.2/BIC-seq"
BICseq_CONFIG=$BICseq_INPUT".config"
BICseq_PREFIX="BICseq.output"

BICseq_DIR=$baseDir"/BICseq"

###
#	Run BIC-seq pipeline
###
if [ ! -d "$BICseqDIR" ]; then
	echo "Running BIC-seq..."
	$BICseq_LOC/BIC-seq.pl $BICseq_CONFIG $BICseq_DIR $BICseq_PREFIX
fi

