#! /bin/bash

cp chedit chedit.bak
if gcc -std='gnu99' chedit.h chedit.c -o chedit -lncurses; then
	rm chedit.bak
else
	mv chedit.bak chedit
fi
