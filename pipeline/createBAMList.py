                                         
#==============================================================================
#title : 		createBAMList.sh								
#author: 		Gryte Satas									
#date:		2013-12-04				
#usage:		createBAMList.sh DIR
#description: Creates a list of BAM pairs in the format required for 
#		processBatchBAM.py. Expects a directory structure as follows:
#		DIR
#			{ID}N
#				dirname
#					normalBAM.bam
#			{ID}T
#				dirname
#					tumorBAM.bam
#		With any number of tumor/normal pairs
#==============================================================================


import os
import sys

pwd = sys.argv[1]
def getDirs(pwd):
	dirs = []
	try:
		for name in os.listdir(pwd):
			fullpath = os.path.join(pwd, name)
			if os.path.isdir(fullpath):
				dirs.append(name)
			dirs.sort()
		return dirs
	except:
		return []

def getBAM(pwd):
	for name in os.listdir(pwd):
		if name.endswith(".bam"):
			return os.path.join(pwd, name)
	return False

dirs = getDirs(pwd)
pairs = []

i = 0
while i < len(dirs)-1:
	if dirs[i][:-1] == dirs[i+1][:-1]:
		pairs.append((dirs[i], dirs[i+1]))
		i += 2
	else:
		i += 1

for pair in pairs:
	normal, tumor = pair
	ID = normal[:-1]
	### TWO levels deep
#	tumorDir = getDirs(os.path.join(pwd,tumor))
#	tumorBAM = normBAM = False
#	if len(tumorDir) > 0: 
#		tumorBAM = getBAM(os.path.join(pwd,tumor,tumorDir[0]))
#	normDir = getDirs(os.path.join(pwd,normal))
#	if len(normDir) > 0:
#		normBAM = getBAM(os.path.join(pwd,normal,normDir[0]))
	### ONE level deep
	tumorBAM = getBAM(os.path.join(pwd,tumor))
	normBAM = getBAM(os.path.join(pwd,normal))



	if tumorBAM and normBAM:
		print tumorBAM
		print normBAM
		print ID

