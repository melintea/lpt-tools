#
# Note: malloc hooks have been deprecated (https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc)
#


LIBBFD_FPIC_INC = /usr/include
LIBBFD_FPIC_LIB = -lbfd

CXX			 = g++

LPT_CXXFLAGS = -ggdb -fPIC -Wall -I../../include -I$(LIBBFD_FPIC_INC) 
LPT_LDFLAGS  = -rdynamic $(LIBBFD_FPIC_LIB) -liberty -lpthread -ldl 



.SUFFIXES: .hpp
vpath %.hpp ../../include
vpath %.cpp .


#
#
#

all : libmemleak.so


.PHONY: clean
clean:
	-rm -rf *.o core* *.core *.so


#
#
#

%.o: %.cpp Makefile
	$(CXX) -c $< -o $@ $(LPT_CXXFLAGS)

#
# libmemleak* & tests
#
LIBMEMLEAK_API_MTRACE = libmemleak.so

LIBMEMLEAK_API_FILES = api_hooks.cpp report.cpp \
					   libmemleak.cpp 

$(LIBMEMLEAK_API_MTRACE): $(LIBMEMLEAK_API_FILES)  Makefile
	$(CXX) $(LPT_CXXFLAGS) -shared $(ATOMIC_TARGET) -o $(LIBMEMLEAK_API_MTRACE)  $(LIBMEMLEAK_API_FILES) $(LPT_LDFLAGS)
