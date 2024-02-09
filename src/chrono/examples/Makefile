#
#
#

FLAGS = -I../../../include -ggdb -std=c++20 -O3

all: barrier1 chronotest chronostats lockfree1 

barrier1: barrier1.cpp Makefile ../../../include/lpt/*.h*
	g++ barrier1.cpp $(FLAGS) -lpthread -o barrier1

chronotest: chronotest.cpp Makefile ../../../include/lpt/*.h*
	g++ chronotest.cpp $(FLAGS) -lpthread -o chronotest

chronostats: stats.cpp Makefile ../../../include/lpt/*.h*
	g++ stats.cpp $(FLAGS) -lpthread -o chronostats

lockfree1: lockfree1.cpp Makefile ../../../include/lpt/*.h*
	g++ lockfree1.cpp $(FLAGS) -lpthread -o lockfree1

clean:
	-rm *.o barrier1 chronotest chronostats lockfree1
