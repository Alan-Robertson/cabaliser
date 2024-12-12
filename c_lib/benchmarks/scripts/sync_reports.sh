#!/bin/bash


if [ $# -ne 2 ]; then
	echo "Missing either target directory and/or identifer"
	echo "Please provide path that will sync with output/ and ident"

	echo "Example: bash sync_reports.sh ~/reports qft_tests_wed"
	exit 1
fi


TARGET_DIR=$1
COMP_IDENT=$($2)
SYNC_DATETIME=$(date  +"%Y%m%d_%H%M%S");
SYNC_DATE=$(date +"%Y%m%d")
OUTPATH="$TARGET_DIR/$SYNC_DATE-$COMP_IDENT-reports"

lscpu | head -n 18 | grep -v 'scaling' > $OUTPATH/cpu_info.txt 

echo "Synchronising with folder"

rsync -r output/$OUTPATH
echo "Synchronised on $SYNC_DATETIME" > $TARGET_DIR/last_sync_date.txt

echo "Completed"
