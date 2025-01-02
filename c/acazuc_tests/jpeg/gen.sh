#!/bin/sh
i=1
while [ "$i" -le 100 ]
do
	./jpeg $1 $i.jpg $i
	i=$((i+1))
done
