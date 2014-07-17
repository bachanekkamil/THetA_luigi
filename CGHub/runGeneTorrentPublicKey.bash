#!/bin/bash

EXPECTED_ARGS=2
E_BADARGS=65

if [ $# -ne $EXPECTED_ARGS ]
then
	echo "Usage: $0 analysis_data_uri output_dir"
	exit $E_BADARGS
fi

#Input Parameters
DOWNLOAD_FILE=$1
OUTPUT_DIR=$2

#Static Parameters
GENE_TORRENT_BIN=/research/compbio/software/CGHub/GeneTorrent-3.3.4-Ubuntu10.x86_64/usr/bin
GENE_TORRENT_CONFS=/research/compbio/software/CGHub/GeneTorrent-3.3.4-Ubuntu10.x86_64/usr/share/GeneTorrent/
KEY_FILE=/research/compbio/software/CGHub/cghub_public.pem

echo -e "Output Directory:" $OUTPUT_DIR
echo -e "File Query:" $DOWNLOAD_FILE


#Run Code - Always use verbose mode
cd $GENE_TORRENT_BIN

echo ./GeneTorrent -v -c $KEY_FILE -d $DOWNLOAD_FILE -p $OUTPUT_DIR --confDir $GENE_TORRENT_CONFS
./GeneTorrent -v -c $KEY_FILE -d $DOWNLOAD_FILE -p $OUTPUT_DIR --confDir $GENE_TORRENT_CONFS
