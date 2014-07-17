import os
import sys
import time
###
#	Two cases:
#		New run, create file
#		Old run, add file
###

filename = sys.argv[1]
config = os.environ['config']

bamTumor = sys.argv[2]
bamNormal = sys.argv[3]

f = open(config)
lines = f.readlines()
f.close()

lines = [line.strip() for line in lines]
lines = [line for line in lines if len(line) > 0]
lines = [line for line in lines if not line.startswith("#")] 

print "Configuration settings:"
for line in lines: print line

f = open(filename, 'a+')
f.write("------------------------------------------\n")
f.write(time.asctime(time.localtime(time.time())))
f.write("\n")
f.write("BAM TUMOR: " + bamTumor + "\n")
f.write("BAM NORMAL: " + bamNormal + "\n")
for line in lines: f.write(line + "\n")
f.write("\n")
f.close()
