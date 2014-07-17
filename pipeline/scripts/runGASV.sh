#!/bin/bash

#==============================================================================
#title :		runGASV.sh
#author:		Gryte Satas
#date:		2013-09-16
#description:	runs BAMtoGASV and GASV
#usage:		runGASV.sh runBAMtoGASV runGASV
#==============================================================================




runBAMtoGASV=true
runGASV=true

mappingQuality=30
MIN_COV=20

memoryOptions="-Xms512m -Xmx16g"
gasvDir="../PipelineSoftware/gasv"

if $runBAMtoGASV; then
	if [ -f "$B2GV_outputN" ]; then
		echo "BAMtoGASV output already exists for normal BAM: $B2GV_outputN"
	else
		echo "Running BAMToGASV on normal BAM..."
		java $memoryOptions -jar $gasvDir/bin/BAMToGASV.jar $bamNormal \
			 -MAPPING_QUALITY $mappingQuality -WRITE_CONCORDANT True\
			 -NOSORT True -OUTPUT_PREFIX $B2GV_prefixN
	fi
	if [ -f "$B2GV_outputT" ]; then
		echo "BAMtoGASV output already exists for tumor BAM: $B2GV_outputT"
	else
		echo "Running BAMToGASV on tumor BAM..."
		java  $memoryOptions -jar $gasvDir/bin/BAMToGASV.jar $bamTumor \
			 -MAPPING_QUALITY $mappingQuality -WRITE_CONCORDANT True\
			 -NOSORT True -OUTPUT_PREFIX $B2GV_prefixT
	fi
fi

if $runGASV; then
	if [ -f "$gasvOutputNormal" ]; then
		echo "GASV output for normal input already exists: $gasvOutputNormal"
	else
		echo "Running GASV for normal input"
		java -jar $software/gasv/bin/GASV.jar  --nocluster --verbose \
			  --maximal --output regions --outputdir $normalDir \
			  --batch $B2GV_prefixN.gasv.in
	fi
	if [ -f "$gasvOutputTumor" ]; then
		echo "GASV output for tumor input already exists: $gasvOutputTumor"
	else
		echo "Running GASV for tumor input"
		java -jar $software/gasv/bin/GASV.jar --cluster --verbose \
			  --maximal --minClusterSize $MIN_COV --output regions \
			  --outputdir $tumorDir --batch $B2GV_prefixT.gasv.in
	fi
fi



