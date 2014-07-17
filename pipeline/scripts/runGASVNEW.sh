baseDIR=$1
normalDir="$baseDIR/NORMALGASV"
tumorDir="$baseDIR/TUMORGASV"

INPUT_NORMAL=$normalDir/NORMAL.lt230.gasv.in.gasv.in
INPUT_TUMOR=$tumorDir/TUMOR.lt230.gasv.in.gasv.in
MIN_COV=20
software="../software/"
#NORMAL
java -jar $software/gasv/bin/GASV.jar  --nocluster --verbose \
	--maximal --output regions --outputdir $normalDir \
	--batch $INPUT_NORMAL
#TUMOR
java -jar $software/gasv/bin/GASV.jar --cluster --verbose \
	 --maximal --minClusterSize $MIN_COV --output regions \
	 --outputdir $tumorDir --batch $INPUT_TUMOR
