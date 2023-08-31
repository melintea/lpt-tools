#
#
#

FLAGS = -I../../include -ggdb -std=c++20 -O3

all: papitest1 papimove

papitest1: papitest1.cpp Makefile ../../include/lpt/*.h*
	g++ papitest1.cpp $(FLAGS) -lpapi -lpthread -o papitest1

papimove: papimove.cpp Makefile ../../include/lpt/*.h*
	g++ papimove.cpp $(FLAGS) -lpapi -lpthread -o papimove

clean:
	-rm papitest1 papimove
