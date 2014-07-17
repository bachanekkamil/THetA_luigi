#!/usr/bin/env python

#==============================================================================
#title :	PairBAMFiles.py
#author: 	Gryte Satas									
#date:		2013-12-17						
#usage: PairBAMFiles.py BarcodeFile (prefix)
#args: 
#		BarcodeFile: File containing lines with the analysisID and bar code
#			tab separted
#==============================================================================

import sys
import os
import glob

filename = sys.argv[1]

if len(sys.argv) > 2: prefix = sys.argv[2]
else: prefix = "./"

with open(filename) as f:
	lines = [s.strip().split() for s in f.readlines()]

def splitBarCode(barcode):
	"""
	For a description of the barcode, see:
	https://wiki.nci.nih.gov/display/TCGA/TCGA+Barcode

	Returns (in order):
		pID: Unique participant ID
		type: 0 for tumor, 1 for normal
	"""

	barcode = barcode.split("-")
	pID = "-".join(barcode[0:3])
	tissueType = int(barcode[3][0])

	return (pID, tissueType)

tumorFiles = {}
normalFiles = {}

# Organize Files into Tumor and Normal
for line in lines:
	pID, tissueType = splitBarCode(line[1])
	if tissueType == 0: tumorFiles[pID] = line[0]
	elif tissueType == 1: normalFiles[pID] = line[0]

# Pair Up
for key in tumorFiles.keys():
	if key in normalFiles:
		tumorLoc = os.path.join(prefix, tumorFiles[key])
		normalLoc = os.path.join(prefix, normalFiles[key])
	
		print glob.glob(tumorLoc+"/*.bam")[0]
		print glob.glob(normalLoc+"/*.bam")[0]
		print key	
