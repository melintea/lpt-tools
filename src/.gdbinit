
set breakpoint pending on

set environment LD_LIBRARY_PATH /opt/lpt/gcc-4.7.0-bin/lib

#set environment LD_PRELOAD      binaries/lib/libmemleak_api_hooks.so
#file  binaries/bin/1001leakseach

set environment MALLOC_TRACE    libmtrace/logs/mtrace.stack.log
set environment LD_PRELOAD      binaries/lib/libmtrace.so
file  binaries/bin/dleaker



b main
b mtrace
b muntrace
b _fini
b error(__ptr_t ptr, const char *msg, const char *file, int line)
b libmemleak::alloc(__ptr_t ptr, memsize_type size)
b libmemleak::free(__ptr_t ptr, memsize_type size)
b libmemleak::realloc(__ptr_t newptr, __ptr_t oldptr, memsize_type size)
b libmemleak::allign(__ptr_t ptr, memsize_type size)
b libmemleak::error(__ptr_t ptr, memsize_type size)
b lpt::stack::detail::bfd::library::~library()

#file  binaries/bin/papitest
b papitest.cpp:163
b call_stack.hpp:536
