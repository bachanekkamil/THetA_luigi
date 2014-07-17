#!/bin/bash

allchr=$(cat $2/data/targets/$3/$4/*_chromosome.txt)

############################################################################		if output files already exist print warning

if [ -e $2/data/targets/$3/$4/GCC/*Ex_GCC.txt ] ; then 
	echo 'Cleaning output folder from old support GCC data!'
	rm -f $2/data/targets/$3/$4/GCC/*Ex_GCC.txt
fi


if [ -e $2"/data/targets/"$3/$4"/GCC/*.GCC.RData" ] ; then 
	echo 'Cleaning output folder from old R packed GCC data!'
	rm -f $2/data/targets/$3/$4/GCC/*.GCC.RData
fi

############################################################################		fasta file indexing if needed

if [ ! -e $5.fai ] ; then 
	samtools faidx $5
fi

############################################################################		target filtering

for i in $allchr
do
	awk -v find=$i '{if ($1==find) print $0}'  $1 | sort -un -k2 > $2/data/targets/$3/$4/.filt_target.txt

############################################################################		GC content calculations
	while read str
	do
		ex_start=$(echo $str | awk '{ print $2}')
		ex_end=$(echo $str | awk '{ print $3}')
		samtools faidx $5 $i:$ex_start-$ex_end | sed '1d' | tr -cd "GgCc" | wc -m >> $2/data/targets/$3/$4/GCC/$i.Ex_GCC.txt
	done < $2/data/targets/$3/$4/.filt_target.txt 
	R --slave --args $2/data/targets/$3/$4/GCC/$i.Ex_GCC.txt,$2/data/targets/$3/$4/GCC,$i < $2/lib/R/FixGC3.R
	rm -f $2/data/targets/$3/$4/GCC/$i.Ex_GCC.txt
done
rm -f $2/data/targets/$3/$4/.filt_target.txt
