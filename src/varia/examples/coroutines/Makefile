#
#
#

FLAGS = -I../../../../include -ggdb -std=c++23 -O0 -fcoroutines -fdump-lang-coro # g++ > 15.1

all: corodebug

corodebug: debug.cpp Makefile ../../../../include/lpt/coroutines/*.h* 
	/opt/gcc152/bin/g++ $(FLAGS) -o corodebug debug.cpp   

debug: corodebug
	LD_LIBRARY_PATH=/opt/gcc152/lib64 cgdb -d /opt/gdb163/bin/gdb ./corodebug
clean:
	-rm *.o corodebug 
