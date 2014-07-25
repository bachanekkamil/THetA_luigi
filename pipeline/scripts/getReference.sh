###
# 	Header files
#
# ../../data/reference_genomes/hg18.fa
# ../../data/reference_genomes/hg19.fa
# ../../data/SNPs/hg18_SNPs.tsv
# ../../data/SNPs/hg19_SNPs.tsv
#
#
#
###

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