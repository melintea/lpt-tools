#
#
#

FLAGS = -I../../../../include -ggdb -std=c++23 -O0 -fcoroutines #-fdump-lang-coro # gcc 15.1

all: corodebug

corodebug: debug.cpp Makefile ../../../../include/lpt/coroutines/*.h* 
	g++ $(FLAGS) -o corodebug debug.cpp   

clean:
	-rm *.o corodebug 
