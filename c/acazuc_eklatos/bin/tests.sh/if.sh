#!/bin/sh

if true
then
	echo OK
else
	echo KO
fi

if false
then
	echo KO
else
	echo OK
fi

if ! false
then
	echo OK
else
	echo KO
fi

if ! true
then
	echo KO
else
	echo OK
fi

if false
then
	echo KO
elif false
then
	echo KO
elif true
then
	echo OK
else
	echo KO
fi

if false
then
	echo KO
elif false
then
	echo KO
elif false
then
	echo KO
else
	echo OK
fi

if true; then echo OK; else echo KO; fi
if false; then echo KO; else echo OK; fi
