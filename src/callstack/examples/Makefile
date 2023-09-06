#
#
#

FLAGS = -I../../../include -ggdb -std=c++20 -O3

all: callstack1

callstack1: exception.cpp Makefile ../../../include/lpt/callstack/*.h* ../../../include/lpt/callstack/detail/*.h*
	g++ $(FLAGS) -o callstack1 exception.cpp -rdynamic -lbfd -lpthread   

clean:
	-rm *.o callstack1 
