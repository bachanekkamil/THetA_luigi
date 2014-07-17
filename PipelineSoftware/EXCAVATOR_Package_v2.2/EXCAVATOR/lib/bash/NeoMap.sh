#!/bin/bash

allchr=$(cat $2/data/targets/$3/$4/*_chromosome.txt)

check=$(echo $allchr | grep "chr" | wc -c)
if (($check == 0)); then

for i in $allchr
do
  
  grep -w "chr"$i $1 | cut -f2,3,4 > $2/.tmp/temp.txt
	R --slave --args $i,$2,$2/data/targets/$3/$4/$4.RData,$2/.tmp/temp.txt,$2/data/targets/$3/$4/MAP < $2/lib/R/NeoMap.R
done

else

for i in $allchr
do
  
	grep -w "$i" $1 | cut -f2,3,4 > $2/.tmp/temp.txt
	R --slave --args "$i",$2,$2/data/targets/$3/$4/$4.RData,$2/.tmp/temp.txt,$2/data/targets/$3/$4/MAP < $2/lib/R/NeoMap.R
done

fi

rm -f $2/.tmp/temp.txt
