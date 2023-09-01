#
#
#

FLAGS = -I../../include -ggdb -std=c++20 -O3

all: papitest1 papimove1 papimove2

papitest1: papitest1.cpp Makefile ../../include/lpt/*.h*
	g++ papitest1.cpp $(FLAGS) -lpapi -lpthread -o papitest1

papimove1: papimove1.cpp Makefile ../../include/lpt/*.h*
	g++ papimove1.cpp $(FLAGS) -lpapi -lpthread -o papimove1

papimove2: papimove2.cpp Makefile ../../include/lpt/*.h*
	g++ papimove2.cpp $(FLAGS) -lpapi -lpthread -o papimove2

clean:
	-rm *.o papitest1 papimove1 papimove2
