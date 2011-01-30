#!/bin/sh

PCAP2BIN=./pcap2bin

if [ $# -ne 1 ]; then
	echo "usage: $0 <file name>" >&2
	exit 1
fi

if [ ! -r $PCAP2BIN ]; then
	echo "pcap2bin file not found" >&2
	exit 1
fi

input_filename=$1
output_filename=${input_filename%.pcap}

proto=0xBEEF
index=0

while true; do
	npacket=`tcpdump -r $input_filename -q vlan and ether proto $proto | wc -l`
	#npacket=`tcpdump -r $input_filename -q ether proto $proto | wc -l`
	
	if [ $npacket -eq 0 ]; then
		break
	fi

	$PCAP2BIN -p $proto $input_filename $output_filename.$index

	proto=`printf "0x%x" $((proto + 1))`
	index=$((index + 1))
done

echo "extracted #$index segments"
