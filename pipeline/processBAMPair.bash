#!/bin/bash

#==============================================================================
#title : 		processBamFiles.sh								
#author: 		Gryte Satas									
#date:		2013-09-10									
#usage:		processBAMPair.sh configFile bamTumor bamNormal prefix
#description:
#	* Creates directory structure for output files in 
#		/data/compbio/publicdata/TCGA_PROCESSED
#	* Extracts header files from bam files
#	* Creates pileup files
#	* VarScan
#	* (Virmid)
#	* BAMtoGASV 
#	* GASV
#	* PREGO 
#	* BIC-seq
#	* THetA
#==============================================================================

if [ $# -ne 4 ]; then
	echo "USAGE: processBAMPair.sh config tumorBam normalBam ID"
	exit 1
fi

#set -e

export config=$1
export bamTumor=$2
export bamNormal=$3
export prefix=$4

source $config
###
#	Create directory structure
###
echo "### Create Directories ###"

echo $baseDir

if [ -d "$baseDir" ]; then
	echo "Base directory already exists: $baseDir"
else
	echo "Creating directory: $baseDir"
	mkdir $baseDir
fi

if [ -d "$normalDir" ]; then
	echo "NORMAL directory already exists: $normalDir"
else
	echo "Creating directory: $normalDir"
	mkdir $normalDir
fi

if [ -d "$tumorDir" ]; then
	echo "TUMOR directory already exists: $tumorDir"
else
	echo "Creating directory: $tumorDir"
	mkdir $tumorDir
fi

echo "-------------------------------------------------"

###
#	Write configuration settings to metadata file
###
echo "### Writing Metadata File ###"

STATUS=$baseDir/$prefix.STATUS
touch $STATUS
python scripts/createMetadata.py $baseDir/$prefix.META $bamTumor $bamNormal

if [ $? -ne 0 ]; then
	echo "Creating Metadata File: FAILURE" > $STATUS
	exit 1
fi
echo "Creating Metadata File: SUCCESS" > $STATUS

	



echo "-------------------------------------------------"
###
# 	Header files
###

if $getHeaders; then
	echo "### Extract Headers ###"
	if [ -f "$normalHeader" ]; then
		  echo "NORMAL header already exists: $normalHeader"
	else
		echo "Extracting header file from normal genome: $normalHeader"
		samtools view -H $bamNormal > $normalHeader

		if [ $? -ne 0 ]; then
			echo "Extracting Normal Header: FAILURE" > $STATUS
			exit 1
		fi
		echo "Extracting Normal Header: SUCCESS" > $STATUS
	fi
	
	if [ -f "$tumorHeader" ]; then
		  echo "TUMOR header already exists: $tumorHeader"
	else
		echo "Extracting header file from tumor genome: $tumorHeader"
		samtools view -H $bamTumor > $tumorHeader
		if [ $? -ne 0 ]; then
			echo "Creating Metadata File: FAILURE" > $STATUS
			exit 1
		fi
		echo "Creating Metadata File: SUCCESS" > $STATUS
	fi
fi

	
echo "-------------------------------------------------"

###
#	Extract reference genome info from header
##
echo "### Identify Reference Genome ###"

if grep -q "GRCh36" $normalHeader ; then
	reference=$fasta_hg18
	cytobands=$cytoBands_hg18
	SNP=$snp_hg18
	echo "Using hg18.fa as reference genome"
elif grep -q "NCBI-human-build36" $normalHeader; then
	reference=$fasta_hg18
	cytobands=$cytoBands_hg18
	SNP=$snp_hg18
	echo "Using hg18.fa as reference genome"
elif grep -q "HG18" $normalHeader; then
	reference=$fasta_hg18
	cytobands=$cytoBands_hg18
	SNP=$snp_hg18
	echo "Using hg18.fa as reference genome"
elif grep -q "NCBI36" $normalHeader; then
	reference=$fasta_hg18
	cytobands=$cytoBands_hg18
	SNP=$snp_hg18
	echo "Using hg18.fa as reference genome"
elif grep -q "HG36.1" $normalHeader; then
	reference=$fasta_hg18
	cytobands=$cytoBands_hg18
	SNP=$snp_hg18
	echo "Using hg18.fa as reference genome"
elif grep -q "GRCh37" $normalHeader; then
	reference=$fasta_hg19
	cytobands=$cytoBands_hg19
	SNP=$snp_hg19
	echo "Using hg19.fa as reference genome"
else
	echo "Identifying Reference Genome: FAILURE" > $STATUS
	echo "Reference genome info not found in header file. Exiting..."
	exit 1
fi
echo "-------------------------------------------------"

###
#	Create pileup files # FOR VARSCAN
###
if $createPileup; then
	echo "### Create Pileup Files ###"
	if [ -f "$normalPileup" ]; then
		echo "NORMAL pileup file already exists: $normalPileup"
	else
		echo "Creating pileup file for normal genome: $normalPileup"
		samtools mpileup -f $reference $bamNormal > $normalPileup
		if [ $? -ne 0 ]; then
			echo "Creating Normal Pileup: FAILURE" > $STATUS
			exit 1
		fi
		echo "Creating Normal Pileup: SUCCESS" > $STATUS
	fi
	
	if [ -f "$tumorPileup" ]; then
		echo "TUMOR pileup file already exists: $tumorPileup"
	else
		echo "Creating pileup file for tumor genome: $tumorPileup\n"
		samtools mpileup -f $reference $bamTumor > $tumorPileup
		if [ $? -ne 0 ]; then
			echo "Creating Tumor Pileup: FAILURE" > $STATUS
			exit 1
		fi
		echo "Creating Tumor Pileup: SUCCESS" > $STATUS

	fi
	echo "-------------------------------------------------"
fi

###
#	Varscan
###
if $runVarScan ; then
	echo "### VarScan ###"
	$scripts/runVarScan.sh 
	if [ $? -ne 0 ]; then
		echo "Running VarScan: FAILURE" > $STATUS
		exit 1
	fi
	echo "Running VarScan: SUCCESS" > $STATUS

	echo "-------------------------------------------------"

fi 

###
#	Virtual SNP array
###
if $runSNP ; then
	echo "### SNP Counts ###"
	$scripts/runVirtualSNPArray.sh $SNP
	if [ $? -ne 0 ]; then
		echo "Running Virtual SNP Array: FAILURE" > $STATUS
		exit 1
	fi
	echo "Running Virtual SNP Array: SUCCESS" > $STATUS

	echo "-------------------------------------------------"
fi
####
##	BAMtoGASV and GASV
####
if $runBAMtoGASV || $runGASV ; then
	echo "### BAMtoGASV and GASV###"
	$scripts/runGASV.sh $runBAMtoGASV $runGASV
	if [ $? -ne 0 ]; then
		echo "Running BAMtoGASV/GASV: FAILURE" > $STATUS
		exit 1
	fi
	echo "Running BAMtoGASV/GASV: SUCCESS" > $STATUS

	echo "-------------------------------------------------"
fi

###
#	PREGO
###

if $runPREGO ; then
	echo "### PREGO ###"
	$scripts/runPREGO.sh $cytobands $baseDir $tumorDir $normalDir 5 $prefix

	if [ $? -ne 0 ]; then
		echo "Running PREGO: FAILURE" > $STATUS
		exit 1
	fi
	echo "Running PREGO: SUCCESS" > $STATUS
	echo "-------------------------------------------------"
fi 

###
#	BICseq and THetA
###
if $runBICseq ; then
	echo "### BICseq ###"

	TUMOR_CONC="${B2GV_prefixT}_$lib.concordant"
	NORMAL_CONC="${B2GV_prefixN}_$lib.concordant"
	
	TUMOR_CONC_CAT="${B2GV_prefixT}*.concordant.cat"
	NORMAL_CONC_CAT="${B2GV_prefixN}*.concordant.cat"

	if [ ! -f $TUMOR_CONC_CAT ]; then
		cat ${B2GV_prefixT}*.concordant > $TUMOR_CONC_CAT
	fi

	if [ ! -f $NORMAL_CONC_CAT ]; then
		cat ${B2GV_prefixN}*.concordant > $NORMAL_CONC_CAT
	fi

	$scripts/runBICseq.sh $TUMOR_CONC_CAT $NORMAL_CONC_CAT

	if [ $? -ne 0 ]; then
		echo "Running BICseq: FAILURE" > $STATUS
		exit 1
	fi
	echo "Running BICseq: SUCCESS" > $STATUS
	
fi

if $runTHetA ; then
	echo "### THetA ###"
	$scripts/runTHetA.sh
fi

