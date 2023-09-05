#
# Configurables
#

# PAPI
PAPI_HOME = /home/amelinte/projects/papi-5.0.1/src

#ATOMIC_TARGET=-march=i486 #x86, x86_64
#ATOMIC_TARGET=-march=amd64


#
# Hands off.
#

LIBBFD_FPIC_INC = /usr/include
LIBBFD_FPIC_LIB = -lbfd

PAPI_LIB  = $(PAPI_HOME)/libpapi.a
PAPI_H	= $(PAPI_HOME)/papi.h


CXX			 = g++


LPT_BINDIR = binaries/bin
LPT_LIBDIR = binaries/lib
LPT_OBJDIR = binaries/obj
LPT_FILES  = 


LPT_CXXFLAGS = -std=c++0x -ggdb -fPIC -Wall -Ilpt/include -I$(PAPI_HOME) -I$(LIBBFD_FPIC_INC) $(CXXFLAGS) $(CFLAGS)
LPT_LDFLAGS  = -rdynamic $(PAPI_LIB) $(LIBBFD_FPIC_LIB) $(LDFLAGS) -liberty -lpthread -ldl 



.SUFFIXES: .hpp
vpath %.hpp lpt/include:lpt/include/lpt
vpath %.cpp lpt/src:tests


#
#
#

# See note in mtrace.cpp as to why this target is excluded.
#	  libmtrace \

all : clean prereq \
	  mallinfo \
	  stacktest \
	  libmemleak_mtrace_hook \
	  papitest
	-echo "*** All Done!"


.PHONY: prereq
prereq:
	mkdir -p $(LPT_BINDIR) $(LPT_LIBDIR) $(LPT_OBJDIR)
	test -f $(PAPI_LIB)
	test -f $(PAPI_H)


.PHONY: clean
clean:
	-rm -rf memleak.* *.o core* *.core $(LPT_BINDIR) $(LPT_LIBDIR) $(LPT_OBJDIR)


archive: clean
	(cd .. && tar -cvzf lpt.tgz lpt && ls -l lpt.tgz && cd lpt)


#
#
#

$(LPT_OBJDIR)/%.o: %.cpp Makefile
	$(CXX) -c $< -o $@ -fPIC

$(LPT_OBJDIR): %.cpp Makefile
	$(CXX) -c $< -o $@ -fPIC


#
# libmtrace & tests
#
LIBMTRACE = $(LPT_LIBDIR)/libmtrace.so
LEAKER	= $(LPT_BINDIR)/dleaker
libmtrace: $(LIBMTRACE)
	$(CXX) -ggdb -Wall -ldl -o $(LEAKER)  tests/leaker.cpp
	@ls -l $(LIBMTRACE)
	@ls -l $(LEAKER)
	-rm libmtrace/logs/*
	MALLOC_TRACE=libmtrace/logs/mtrace.stack.log  LD_PRELOAD=$(LIBMTRACE)  $(LEAKER)
	-mtrace $(LEAKER) libmtrace/logs/mtrace.stack.log > libmtrace/logs/mtrace.stack.leaks.log
	MALLOC_TRACE=libmtrace/logs/mtrace.plain.log  $(LEAKER)
	-mtrace $(LEAKER) libmtrace/logs/mtrace.plain.log > libmtrace/logs/mtrace.plain.leaks.log
	@ls -l libmtrace/logs/*
	@echo "*** libmtrace: done"

LIBMTRACE_FILES = libmtrace/mtrace.cpp libmtrace/libmtrace.cpp $(LPT_FILES)
$(LIBMTRACE): $(LIBMTRACE_FILES)  Makefile
	$(CXX) $(LPT_CXXFLAGS) -shared $(ATOMIC_TARGET) -o $(LIBMTRACE)  $(LIBMTRACE_FILES) $(LPT_LDFLAGS)



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

LIBMEMLEAK_MTRACE_FILES = libmemleak/mtrace_hooks.cpp libmemleak/report.cpp \
						  libmemleak/libmemleak.cpp \
						  $(LPT_FILES)
$(LIBMEMLEAK_MTRACE): $(LIBMEMLEAK_MTRACE_FILES)  Makefile
	$(CXX) $(LPT_CXXFLAGS) -shared $(ATOMIC_TARGET) -o $(LIBMEMLEAK_MTRACE)  $(LIBMEMLEAK_MTRACE_FILES) $(LPT_LDFLAGS)


LIBMEMLEAK_API_FILES = libmemleak/api_hooks.cpp libmemleak/report.cpp \
					   libmemleak/libmemleak.cpp \
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


#
# PAPI tests
#

PAPITEST = $(LPT_BINDIR)/papitest
papitest: $(PAPITEST)
	PAPI_DEBUG=SUBSTRATE $(PAPITEST)
	@echo "*** papitest: done"

PAPITEST_FILES = tests/papitest.cpp $(LPT_FILES)
$(PAPITEST) : tests/papitest.cpp $(PAPITEST_FILES) papi.hpp Makefile
	@echo "*** papitest"
	$(CXX) $(LPT_CXXFLAGS) -o $(PAPITEST) $(PAPITEST_FILES) $(LPT_LDFLAGS)


#
# Call stack tests
#

STACKTEST = $(LPT_BINDIR)/stacktest
stacktest: $(STACKTEST)
	$(STACKTEST)
	@echo "*** stacktest: done"

STACKTEST_FILES = tests/stack.cpp $(LPT_FILES)
$(STACKTEST) : tests/stack.cpp $(STACKTEST_FILES) Makefile
	@echo "*** stacktest"
	$(CXX) $(LPT_CXXFLAGS) -o $(STACKTEST) $(STACKTEST_FILES) $(LPT_LDFLAGS)

