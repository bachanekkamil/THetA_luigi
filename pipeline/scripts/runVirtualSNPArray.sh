#!/bin/bash

#==============================================================================
#title :	runVirtualSNPArray.sh
#author:	Gryte Satas
#date:	2013-09-20
#description: runVirtualSNPArray.sh SNP
#==============================================================================
config="./configShell.txt"
source $config
set -e
SNP=$1

snpconfig="/tmp/$RANDOM"

touch $snpconfig
echo "SNP_FILE="$SNP > snpconfig
echo "OUTPUT_PREFIX=$baseDir/snp" >> snpconfig
echo "BAM_FILE="$bamNormal >> snpconfig
echo "BAM_FILE="$bamTumor >> snpconfig

pushd software/SNP/bin
java -classpath ".:../lib/sam-1.78.jar" getAlleleCounts $snpconfig
popd
rm $snpconfig
