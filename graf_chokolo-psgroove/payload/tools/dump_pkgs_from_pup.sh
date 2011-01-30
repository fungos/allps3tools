#!/bin/sh

LOCATE_PUP_FILES=./locate_pup_files

if [ $# -ne 1 ]; then
	echo "usage: $0 <file name>" >&2
	exit 1
fi

if [ ! -r $LOCATE_PUP_FILES ]; then
	echo "locate_pup_files file not found" >&2
	exit 1
fi

input_filename=$1

echo "locating packages ..."

$LOCATE_PUP_FILES $input_filename |
while read line; do
	offset=`echo "$line" | awk '{ print $1 }'`
	size=`echo "$line" | awk '{ print $2 }'`
	output_filename=`echo "$line" | awk '{ print $3 }'`

	echo "extracting package $output_filename (offset=$offset, size=$size) ..."

	dd if=$input_filename of=$output_filename bs=1 skip=$(($offset)) count=$(($size))
done
