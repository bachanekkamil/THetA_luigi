#!/bin/bash

set -e

OUT_DIR=$1
bamNormal=$2
bamTumor=$3
REF_ASSEM=$4
THIS_DIR=$5
NAME=$6

memoryOptions="-Xms512m -Xmx32g"

snpconfig=$OUT_DIR/snpconfig.txt

SNP_FILE=""

if [ "$REF_ASSEM" == "hg19" ];then
	SNP_FILE=$THIS_DIR/1000G_hg19_snps.tsv
else
	SNP_FILE=$THIS_DIR/AffySNP6.0_hg18.txt.formatted
fi

touch $snpconfig
echo "SNP_FILE="$SNP_FILE > $snpconfig
#The output prefix also codes for the path to the file.
echo "OUTPUT_PREFIX=$OUT_DIR/$NAME" >> $snpconfig
echo "BAM_FILE="$bamNormal >> $snpconfig
echo "BAM_FILE="$bamTumor >> $snpconfig

# echo "================"
# echo "PARAMATERS"
# echo "================"
# cat $snpconfig
# echo "================"

#Run SNP Counting script
pushd PipelineSoftware/virtualSNPArray/bin
java $memoryOptions -classpath ".:../jars/sam-1.78.jar" AlleleCounts/getAlleleCounts $snpconfig
popd
rm $snpconfig

SNP_FILE_LOC=$(ls $OUT_DIR/*.BAlleleFreqs)

pushd pipeline/scripts
python filterSNPs.py SNP_FILE_LOC
popd

#Plot results
