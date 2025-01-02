#!/bin/sh
for a in a b c $HOME
do
	echo $a
done

for a in a b; do echo $a; done

for a
do
	echo $a
done
