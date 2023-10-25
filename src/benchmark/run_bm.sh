#!/bin/bash
set -e
set -x

#
# Google benchmark wrapper
# Usage: ./run_bm file.cpp
# 

googleBenchmark=${HOME}/work/benchmark

target=$(basename -s \.cpp $1)

g++ $1 -std=c++20 \
  -isystem ${googleBenchmark}/include \
  -L${googleBenchmark}/build/src \
  -L${googleBenchmark}/build/lib \
  -lbenchmark -lpthread \
  -o ${target} 

./${target} --benchmark_counters_tabular=true #--benchmark_repetitions=3
rm ./${target}
