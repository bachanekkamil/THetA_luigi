#!/bin/bash

set -e
# BICseq_LOC="../../PipelineSoftware/BICseq/PERL_pipeline/BICseq_1.1.2/BIC-seq"
#BICseq_CONFIG=$BICseq_INPUT".config"
#BICseq_PREFIX="BICseq.output"
# BICseq_DIR=$baseDir"/BICseq"

BICSEG_LOC=$1 # BICseqToTHetA_INPUT=$BICseq_DIR"/output/"$PREFIX".bicseg"
PREFIX=$2
THETA_OUTPUT_DIR=$3
READ_DEPTH_FILE_LOC=$4

THetA_PREFIX=$PREFIX


echo $PWD
###
#	Run BicSeqToTHetA to create THetA input
###
#if [ ! -f "$THetA_INPUT" ]; then
echo "Running BICSeqToTHetA..."
java -jar PipelineSoftware/theta/bin/runBICSeqToTHetA.jar $BICSEG_LOC -OUTPUT_PREFIX $THetA_PREFIX -MIN_LENGTH 100
#fi
#Move _min_processed (Theta input) files to the Theta directory
mv *_processed* $THETA_OUTPUT_DIR

THetA_INPUT=$THETA_OUTPUT_DIR/$PREFIX.min.100_processed

# THetA_INPUT="$THetA_PREFIX.all_processed"
# THetA_OUTPUT="$THETA_OUTPUT/$PREFIX.results"

###
#	Run THetA
###
#if [ ! -f "$THetA_OUTPUT" ]; then
echo "Running THetA..."
./PipelineSoftware/theta/bin/RunTHetA $THetA_INPUT -d $THETA_OUTPUT_DIR -p $PREFIX --READ_DEPTH_FILE $READ_DEPTH_FILE_LOC --NUM_PROCESSES 16
#else
	#echo "THetA output already exists: $THetA_OUTPUT"
#fi



#=================Old ====================

# source $config
# set -e
# BICseq_LOC="./software/BICseq/PERL_pipeline/BICseq_1.1.2/BIC-seq"
# BICseq_CONFIG=$BICseq_INPUT".config"
# BICseq_PREFIX="BICseq.output"
# BICseq_DIR=$baseDir"/BICseq"

# BICseqToTHetA_INPUT=$BICseq_DIR"/"$BICseq_PREFIX".bicseg"

# THetA_PREFIX="$baseDir/THetA.input"

# ###
# #	Run BicSeqToTHetA to create THetA input
# ###
# if [ ! -f "$THetA_INPUT" ]; then
# 	echo "Running BICSeqToTHetA..."
# 	java -jar software/theta/bin/runBICSeqToTHetA.jar $BICseqToTHetA_INPUT -OUTPUT_PREFIX $THetA_PREFIX -MIN_LENGTH 100
# fi


# THetA_INPUT="$THetA_PREFIX.all_processed"
# THetA_PREFIX="THetA.output"
# THetA_OUTPUT="$baseDir/$THetA_OUTPUT.results"

# ###
# #	Run THetA
# ###
# if [ ! -f "$THetA_OUTPUT" ]; then
# 	echo "Running THetA..."
# 	software/theta/bin/RunTHetA $THetA_INPUT -d $baseDir -p $THetA_PREFIX	
# else
# 	echo "THetA output already exists: $THetA_OUTPUT"
# fi



