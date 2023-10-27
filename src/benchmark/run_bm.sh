#!/bin/bash
set -e
set -x

#
# Google benchmark wrapper
# Usage: ./run_bm file.cpp
# 

googleBenchmark=${HOME}/work/benchmark

target=$(basename -s \.cpp $1)

#TODO: -mavx2 
g++ $1 -std=c++20 -g -O3 \
  -Wall -Wextra -Werror -pedantic -Wno-deperecated-volatile \
  -isystem ${googleBenchmark}/include \
  -L${googleBenchmark}/build/src \
  -L${googleBenchmark}/build/lib \
  -lbenchmark -lpthread \
  -o ${target} 

./${target} --benchmark_counters_tabular=true #--benchmark_repetitions=3
#perf c2c record -g ./${target} --benchmark_counters_tabular=true #--benchmark_repetitions=3
#perf c2c report 
#rm perf.data

rm ./${target}
