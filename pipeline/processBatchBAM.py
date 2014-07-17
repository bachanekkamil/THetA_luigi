#!/usr/bin/env python

#==============================================================================
#title : 		processBatchBAM.sh								
#author: 		Gryte Satas									
#date:		2013-12-02									
#usage:		processBatchBAM.sh BAMList config
#description: Takes in a file with a list of BAM pairs and runs processBAMPair
#				on each pair. For each pair, there should be 3 lines in the 
#				file, in the following order
#			/path/to/TUMOR.bam
#			/path/to/NORMAL.bam
#			ID
#				where ID is what you want the name of the directory created to
#				be. Blank lines and lines starting with "#" are ignored
#==============================================================================

import sys
from subprocess import call
filename = sys.argv[1]
config = sys.argv[2]

with open(filename) as f:
	lines = f.readlines()
	
lines = [line.strip() for line in lines if len(line.strip()) > 0 and not line.startswith("#")]
print lines
if len(lines)%3 != 0:
	print "Incorrect number of lines. Found", len(lines),"lines. File should contain exactly 3 lines per pair (excluding blank and comment lines)"
	quit()
else:
	print "Found", len(lines)/3, "pairs."


for i in range(0, len(lines), 3):
	tumor = lines[i].strip()
	normal = lines[i+1].strip()
	ID = lines[i+2].strip()
	print "Calling processBAMPair.bash with the following arguments:"
	print "\tconfig:", config
	print "\tBAM Tumor:", tumor
	print "\tBAM Normal:", normal
	print "\tID:", ID
	call(["./processBAMPair.bash", config, tumor, normal, ID])



