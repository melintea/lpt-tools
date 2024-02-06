#!/bin/bash
set -e
set -x

#
# Google benchmark wrapper
# Usage: ./gbench file.cpp
# 

if [ $# -ne 1 ]; then
  echo "Usage: `basename $0` <cpp-file>"
  exit 1
fi

compiler=g++

googleBenchmark=${HOME}/work/benchmark
lptInc=../../include

target=$(basename -s \.cpp $1)

#TODO: -mavx2 
${compiler} $1 -std=c++20 -g -O3 \
  -Wall -Wextra -Werror -pedantic -Wno-deprecated-volatile \
  -isystem ${googleBenchmark}/include \
  -isystem ${lptInc} \
  -L${googleBenchmark}/build/src \
  -L${googleBenchmark}/build/lib \
  -lbenchmark -lpthread \
  -o ${target} 
${compiler} --version

./${target} --benchmark_counters_tabular=true #--benchmark_repetitions=3
#perf c2c record -g --all-user --call-graph ./${target} --benchmark_counters_tabular=true #--benchmark_repetitions=3
#perf c2c report -NN -g --call-graph
#perf lock record -g --all-user --call-graph dwarf,8192
#perf lock report --combine-locks --threads
#rm perf.data

rm ./${target}
