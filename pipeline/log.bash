#!/bin/bash

#==============================================================================
#title : 		log.bash								
#author: 		Gryte Satas									
#date:		2013-12-05									
#usage:		log.bash logfile progName program args1 ... argN
#params:
#		logfile: path to logfile
#		progName: Name the program will be logged as 
#		program: Full path to program
#		args: all arguments to program
#description:
#==============================================================================


logfile=$1
progName=$2
program=$3
args=${@:4}

echo $args

echo `date` $progName START >> $logfile

$program $args

if [ $? -eq 0 ]; then
   echo `date` $progName FINISH >> $logfile
else
   echo `date` $progName ERROR $1 >> $logfile
fi 
