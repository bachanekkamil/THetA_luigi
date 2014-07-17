#!/bin/bash

#==============================================================================
#title :	runTHetA.sh
#author:	Gryte Satas
#date:	2013-09-16
#description:	runs BIC-seq and THetA
#usage:	runTHetA tumorConcordant normalConcordant 
#==============================================================================

source $config
set -e
BICseq_LOC="./software/BICseq/PERL_pipeline/BICseq_1.1.2/BIC-seq"
BICseq_CONFIG=$BICseq_INPUT".config"
BICseq_PREFIX="BICseq.output"
BICseq_DIR=$baseDir"/BICseq"

BICseqToTHetA_INPUT=$BICseq_DIR"/"$BICseq_PREFIX".bicseg"

THetA_PREFIX="$baseDir/THetA.input"

###
#	Run BicSeqToTHetA to create THetA input
###
if [ ! -f "$THetA_INPUT" ]; then
	echo "Running BICSeqToTHetA..."
	java -jar software/theta/bin/runBICSeqToTHetA.jar $BICseqToTHetA_INPUT -OUTPUT_PREFIX $THetA_PREFIX -MIN_LENGTH 100
fi


THetA_INPUT="$THetA_PREFIX.all_processed"
THetA_PREFIX="THetA.output"
THetA_OUTPUT="$baseDir/$THetA_OUTPUT.results"

###
#	Run THetA
###
if [ ! -f "$THetA_OUTPUT" ]; then
	echo "Running THetA..."
	software/theta/bin/RunTHetA $THetA_INPUT -d $baseDir -p $THetA_PREFIX	
else
	echo "THetA output already exists: $THetA_OUTPUT"
fi



