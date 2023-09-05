#
#  Copyright 2011 Aurelian Melinte. 
#  Released under LGPL 3.0 or later. 
#
#  mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
#


all: tests

dleaker: leaker.cpp Makefile
	g++ -ggdb -Wall -ldl -o dleaker  leaker.cpp 

tests: dyntest stacktest
	-mkdir logs
	ls -l logs/*

dyntest: dleaker libmtrace.so
	MALLOC_TRACE=logs/mtrace.plain.log ./dleaker
	-mtrace dleaker logs/mtrace.plain.log > logs/mtrace.plain.leaks.log

stacktest: dleaker libmtrace.so
	MALLOC_TRACE=logs/mtrace.stack.log LD_PRELOAD=./libmtrace.so ./dleaker
	-mtrace dleaker logs/mtrace.stack.log > logs/mtrace.stack.leaks.log

clean:
	-rm sleaker dleaker  libmtrace.so  core*
	ls -l logs/*



