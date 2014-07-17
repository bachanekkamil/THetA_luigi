#!/bin/bash

#==============================================================================
#title : 		runVarScan.sh								
#author: 		Gryte Satas									
#date:		2013-09-16									
#description: 	runs VarScan 
#usage:		runVarScan.sh
#==============================================================================

source $config
set -e

varscanOutput=$baseDir"/output.varscan"

if [ -f "$varscanOutput.indel" ] && [ -f "$varscanOutput.snp" ] ; then
	echo "VarScan output already exists: $varscanOutput"
else
	echo "Running Varscan..."
	java -jar $software/VarScan.v2.3.6.jar somatic $normalPileup \
		$tumorPileup $varscanOutput
fi
	
