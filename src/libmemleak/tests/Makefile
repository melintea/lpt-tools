#
# Test executables for libmemleak/libmemtrace
#

CXX			 = g++


LPT_CXXFLAGS = -ggdb -fPIC -Wall -I../../../include $(CXXFLAGS) $(CFLAGS)
LPT_LDFLAGS  = -rdynamic $(LDFLAGS) -liberty -lpthread -ldl 



.SUFFIXES: .hpp
vpath %.hpp ../../../include
vpath %.cpp .


#
# mallinfo\: deprecated __free_hook/__malloc_hook/etc hooks
#

all : clean \
	  leaker \
	  1001leaksearch
	-echo "*** All Done!"


.PHONY: clean
clean:
	-rm -rf memleak.* *.o core* *.core mallinfo stacktest leaker 1001leaksearch

#
#
#

%.o: %.cpp Makefile
	$(CXX) -c $< -o $@ $(LPT_CXXFLAGS)


#
# Tests for libmemleak & libmtrace.  Usage:
#	LD_PRELOAD=lib.so  executable
#
leaker: leaker.cpp Makefile
	$(CXX) $(LPT_CXXFLAGS) -o leaker leaker.cpp  $(LPT_LDFLAGS)

1001leaksearch: 1001leakseach.cpp Makefile
	$(CXX) $(LPT_CXXFLAGS) -o 1001leaksearch 1001leakseach.cpp  $(LPT_LDFLAGS)

mallinfo: mallinfo.cpp Makefile
	$(CXX) $(LPT_CXXFLAGS) -o mallinfo mallinfo.cpp  $(LPT_LDFLAGS)


