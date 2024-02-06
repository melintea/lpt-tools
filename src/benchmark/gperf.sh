#!/bin/bash
set -e
set -x

#
# Google perf tools wrapper
# https://github.com/gperftools/gperftools
#
# Usage: ./gperf file.cpp
#
# Here are some of the most important variables:
#   HEAPPROFILE=<pre> -- turns on heap profiling and dumps data using this prefix
#   HEAPCHECK=<type>  -- turns on heap checking with strictness 'type'
#   CPUPROFILE=<file> -- turns on cpu profiling and dumps data to this file.
#   PROFILESELECTED=1 -- if set, cpu-profiler will only profile regions of code
#                        surrounded with ProfilerEnable()/ProfilerDisable().
#   CPUPROFILE_FREQUENCY=x-- how many interrupts/second the cpu-profiler samples.
#   PERFTOOLS_VERBOSE=<level> -- the higher level, the more messages malloc emits
#   MALLOCSTATS=<level>    -- prints memory-use stats at program-exit
# 

if [ $# -ne 1 ]; then
  echo "Usage: `basename $0` <cpp-file>"
  exit 1
fi

compiler=g++

target=$(basename -s \.cpp $1)
profdata=${target}.prof.cpu.data

#TODO: -mavx2 
${compiler} $1 -std=c++20 -g -O3 \
  -Wall -Wextra -Werror -pedantic -Wno-deprecated-volatile \
  -pedantic \
  -Wl,--no-as-needed -lprofiler -Wl,--as-needed \
  -lpthread \
  -o ${target} 
${compiler} --version

CPUPROFILE=${profdata} ./${target} 
google-pprof --text ./${target} ${profdata}

rm ./${target}
rm ${profdata}
