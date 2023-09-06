#
# malloc hooks have been deprecated (https://stackoverflow.com/questions/17803456/an-alternative-for-the-deprecated-malloc-hook-functionality-of-glibc)
# Configurables
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

# See note in mtrace.cpp as to why this target is excluded.
#	  libmtrace \

all : libmemleak_mtrace_hook 
	-echo "*** All Done!"


.PHONY: clean
clean:
	-rm -rf *.o core* *.core *.so


#
#
#

%.o: %.cpp Makefile
	$(CXX) -c $< -o $@ -fPIC

#
# libmtrace & tests
#
LIBMTRACE = libmtrace.so
LEAKER    = dleaker

#
# libmemleak_mtrace_* & tests
#
LIBMEMLEAK_MTRACE	 = $(LPT_LIBDIR)/libmemleak_mtrace_hooks.so
LIBMEMLEAK_API_MTRACE = $(LPT_LIBDIR)/libmemleak_api_hooks.so
LEAKER_1001		   = $(LPT_BINDIR)/1001leakseach
libmemleak_mtrace_hook:  $(LIBMEMLEAK_MTRACE) $(LIBMEMLEAK_API_MTRACE)
	$(CXX) -ggdb -Wall -std=c++11 -lpthread -ldl -o $(LEAKER_1001)  tests/1001leakseach.cpp
	@ls -l $(LIBMEMLEAK_MTRACE)
	@ls -l $(LEAKER_1001)
	#LD_PRELOAD=$(LIBMEMLEAK_MTRACE)	  $(LEAKER_1001)
	LD_PRELOAD=$(LIBMEMLEAK_API_MTRACE)  $(LEAKER_1001)
	@echo "*** libmtrace: done"

LIBMEMLEAK_MTRACE_FILES = mtrace_hooks.cpp report.cpp \
						  libmemleak.cpp \
						  $(LPT_FILES)
$(LIBMEMLEAK_MTRACE): $(LIBMEMLEAK_MTRACE_FILES)  Makefile
	$(CXX) $(LPT_CXXFLAGS) -shared $(ATOMIC_TARGET) -o $(LIBMEMLEAK_MTRACE)  $(LIBMEMLEAK_MTRACE_FILES) $(LPT_LDFLAGS)


LIBMEMLEAK_API_FILES = api_hooks.cpp report.cpp \
					   libmemleak.cpp \
						$(LPT_FILES)
$(LIBMEMLEAK_API_MTRACE): $(LIBMEMLEAK_API_FILES)  Makefile
	$(CXX) $(LPT_CXXFLAGS) -shared $(ATOMIC_TARGET) -o $(LIBMEMLEAK_API_MTRACE)  $(LIBMEMLEAK_API_FILES) $(LPT_LDFLAGS)



#
# mallinfo/meminfo tests
#

MALLINFO = $(LPT_BINDIR)/mallinfo
mallinfo: $(MALLINFO)
	@ls -l $(LPT_BINDIR)/*
	$(MALLINFO)
	@echo "*** mallinfo: done"

$(MALLINFO) : tests/mallinfo.cpp Makefile
	@echo "*** mallinfo"
	$(CXX) $(LPT_CXXFLAGS) -o $(MALLINFO) tests/mallinfo.cpp  $(LPT_LDFLAGS)

#obj_mallinfo = $(addprefix $(PLATFORM)/obj/, mallinfo.o)
#
#mallinfo: $(obj_mallinfo) Makefile
#	$(CXX) -o $(PLATFORM)/bin/mallinfo  $(obj_mallinfo)
#	$(PLATFORM)/bin/mallinfo


