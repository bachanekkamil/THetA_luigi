#!/bin/bash

#==============================================================================
#title :	emailLog.bash
#author:	Gryte Satas
#date:	2013-12-05
#description: Sends an email to the address listed with a notification that
#			prefix finished running, and the contents of the logfile
#usage:	emailLog.bash logfile prefix
#==============================================================================

set -e
EMAIL="statusLogPDC@gmail.com"

logtext=$1
prefix=$2

if grep --quiet "ERROR" $logtext ; then
	subject="${prefix} Finished WITH ERROR(s)"
else
	subject="${prefix} Finished without errors"
fi

cat $logtext | mail -s "$subject" "$EMAIL"
