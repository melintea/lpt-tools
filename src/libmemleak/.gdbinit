
set environment LD_LIBRARY_PATH /opt/lpt/gcc-4.7.0-bin/lib
set environment LD_PRELOAD      ../binaries/lib/libmemleak_mtrace_hook.so

file  ../binaries/bin/1001leakseach

set breakpoint pending on
b mtrace()
b _fini()
b /opt/lpt/gcc-4.7.0/i686-pc-linux-gnu/libstdc++-v3/include/fstream:710

