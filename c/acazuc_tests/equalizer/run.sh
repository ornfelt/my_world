#!/bin/sh
if [ $# -lt 1 ]
then
	echo "run.sh file.raw"
	exit 1
fi

make && ./equalizer "$1" | pacat --channels=1 --format=float32ne --latency-msec=5
