import sys

#==============================================================================
#title :	filterSNPFile.py
#author:	Gryte Satas
#date:	2013-09-24
#description: Goes through SNP file and removes duplicates, negative strand,
#	and indels.
#	Input file must have the following columns:
#		Chrom ChromStart ChromEnd Name(rsID) Strand RefNCBI Observed
#	Output file will have the folwing columns:
#		rsID Chrom Position Strand RefAllele MutAllele
#usage: python filterSNPFile.py inputFile outputFile
#==============================================================================

if len(sys.argv) != 3:
	print "Usage: python filterSNPFile.py inputFile outputFile" 
	exit()

inputF = sys.argv[1]
outputF = sys.argv[2]

print "Arguments are:\n\tInput File:", inputF,"\n\tOutput File:", outputF

f = open(inputF)
lines = f.readlines()
f.close()

lines = lines[1:] #Toss out header

def processLine(line):
	lineDict = {}
	parts = [p.strip() for p in line.split('\t')]
	lineDict['chrom'] = parts[0]
	lineDict['chromStart'] = int(parts[1])
	lineDict['chromEnd'] = int(parts[2])
	lineDict['name'] = parts[3]
	lineDict['strand'] = parts[4]
	lineDict['refNCBI'] = parts[5]
	lineDict['observed'] = parts[6]
	return lineDict
def formatLine(ld):
	return "\t".join([ld['name'],ld['chrom'],str(ld['chromStart']),ld['strand'],\
			    ld['refNCBI'], ld['observed']])+"\n"

#Filter out duplicates, anything on the neg strand and indels
newLines = []
prevChrom, prevStart, prevEnd = (0,0,0)
for line in lines:
	d = processLine(line)

	if d['strand'] != '+':
		continue #neg strand
	if d['chromStart'] == prevStart and d['chromEnd'] == prevEnd \
		and d['chrom'] == prevChrom:
		continue #duplicate
	if (d['chromEnd'] - d['chromStart']) != 1:
		continue #indel

	newLines.append(line)
	prevChrom = d['chrom']
	prevStart = d['chromStart']
	prevEnd   = d['chromEnd']

f = open(outputF, 'w')
f.write("#ID\tchrom\tpos\tstrand\trefAllele\tmutAllele\n")
for line in newLines:
	  f.write(formatLine(processLine(line)))
f.close()
