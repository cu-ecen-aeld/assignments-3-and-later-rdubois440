#!/bin/sh

writefile=$1
writestr=$2
dirname="$(dirname $1)"

echo number of arguments is $#
echo writefile is $filesdir
echo writestr is $filesdir
echo dirname is $dirname

if [ "$#" -ne 2 ]
then
	echo 2 arguments are required
	return 1
fi

if [ ! -d "$dirname" ]
then
	echo directory must be created $dirname
	mkdir -p $dirname
	if [ "$?" -ne 0 ]
	then
		echo directory could not be created $dirname
		exit 1
	fi
fi

echo $writestr > $writefile
if [ "$?" -ne 0 ]
then
	echo file could not be created $writefile
	exit 1
fi





