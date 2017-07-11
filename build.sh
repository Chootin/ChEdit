#! /bin/bash

if [ $# -ne 1 ]; then
	echo "Please provide exactly one argument - ./build.sh 
<program>"
	exit
fi

gcc $1.c -o $1 -lncurses
