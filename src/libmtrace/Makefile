#
#  Copyright 2011 Aurelian Melinte. 
#  Released under LGPL 3.0 or later. 
#
#  mtrace()/muntrace() & tools.  Based on glibc 2.6.1. 
#  smalloc hooks have been deprecated (https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc)
#

#
# Change these for you platform/environment
#
ATOMIC_TARGET=-march=x86-64


all: libmtrace.so

libmtrace.so: mtrace.cpp libmtrace.cpp Makefile
	g++ -I../../include -ggdb -Wall -shared -fPIC $(ATOMIC_TARGET) -lpthread -ldl -o libmtrace.so  mtrace.cpp libmtrace.cpp  

clean:
	-rm sleaker dleaker  libmtrace.so  core*



