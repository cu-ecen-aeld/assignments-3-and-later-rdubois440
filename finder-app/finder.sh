#!/bin/sh

filesdir=$1
searchstr=$2

#echo filesdir is $filesdir
#echo searchstr is $searchstr
#echo number of arguments is $#

if [ "$#" -ne 2 ]
then
	#echo 2 arguments are required
	return 1
fi

if [ ! -d "$1" ]
then
	#echo $1 is not a directory
	return 1
fi

filecount="$(find $filesdir -type f | wc -l)"
#echo The number of files is $filecount

matchinglines="$(find $filesdir -type f  | xargs grep $searchstr 2>/dev/null | wc -l)"
#echo The number of matching lines is $matchinglines

echo The number of files are ${filecount} and the number of matching lines are ${matchinglines}






