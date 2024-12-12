#!/bin/bash


if [ $# -ne 1 ]; then
	echo "Missing target directory, please provide path that will sync with output/"
	exit 1
fi


TARGET_DIR=$1
SYNC_DATETIME=$(date  +"%Y%m%d_%H%M%S");
SYNC_DATE=$(date +"%Y%m%d")
COMP_HOST=$(whoami)

rsync -r output/ $TARGET_DIR/$SYNC_DATE-$COMP_HOST-reports

echo "Synchronised on $SYNC_DATETIME" > $TARGET_DIR/last_sync_date.txt
