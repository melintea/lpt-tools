#
#  Copyright 2011 Aurelian Melinte. 
#  Released under LGPL 3.0 or later. 
#
#  mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
#

#
# Change these for you platform/environment
#
ATOMIC_TARGET=-march=i486


all: tests

libmtrace.so: mtrace.cpp libmtrace.cpp Makefile
	g++ -ggdb -Wall -shared -fPIC $(ATOMIC_TARGET) -lpthread -ldl -o libmtrace.so  mtrace.cpp libmtrace.cpp  

dleaker: leaker.cpp Makefile
	g++ -ggdb -Wall -ldl -o dleaker  leaker.cpp 

tests: dyntest stacktest
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

archive:
	cd .. && tar cvf memtrace.v3.tar memtrace.v3 && cd memtrace.v3
	ls -l ../memtrace.*.tar


