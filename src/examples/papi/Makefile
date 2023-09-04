#
#
#

FLAGS = -I../../../include -ggdb -std=c++20 -O3

all: papitest1 papimove1 papimove2 papimove3

papitest1: papitest1.cpp Makefile ../../../include/lpt/papi/*.h*
	g++ papitest1.cpp $(FLAGS) -lpapi -lpthread -o papitest1

papimove1: papimove1.cpp Makefile ../../../include/lpt/papi/*.h*
	g++ papimove1.cpp $(FLAGS) -lpapi -lpthread -o papimove1

papimove2: papimove2.cpp Makefile ../../../include/lpt/papi/*.h*
	g++ papimove2.cpp $(FLAGS) -lpapi -lpthread -o papimove2

papimove3: papimove3.cpp Makefile ../../../include/lpt/papi/*.h*
	g++ papimove3.cpp $(FLAGS) -lpapi -lpthread -o papimove3

clean:
	-rm *.o papitest1 papimove1 papimove2 papimove3
