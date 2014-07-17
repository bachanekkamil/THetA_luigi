baseDir="$outputLoc/$prefix"
normalDir="$baseDir/NORMAL"
tumorDir="$baseDir/TUMOR"

metaFile="$baseDir/$prefix.META"
normalHeader="$normalDir/$prefix.NORMAL.header"
tumorHeader="$tumorDir/$prefix.TUMOR.header"

normalPileup=$normalDir/$prefix".NORMAL.pileup"
tumorPileup=$tumorDir/$prefix".TUMOR.pileup"

varscanOutput="$baseDir/output.varscan"

B2GV_prefixN="$normalDir/NORMAL"
B2GV_prefixT="$tumorDir/TUMOR"
B2GV_outputT="$B2GV_prefixT.gasv.in"
B2GV_outputN="$B2GV_prefixN.gasv.in"
gasvOutputNormal=$B2GV_outputN".noclusters"
gasvOutputTumor=$B2GV_outputT".clusters"

pregoDir="$baseDir/prego"
