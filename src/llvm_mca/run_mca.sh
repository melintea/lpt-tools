#!/bin/bash
set -e
set -x

#
# LLVM MCA wrapper
# Usage: ./run_mca file.cpp
# 

if [ $# -ne 1 ]; then
  echo "Usage: `basename $0` <cpp-file>"
  exit 1
fi

googleBenchmark=${HOME}/work/benchmark
lptInc=../../include

target=$(basename -s \.cpp $1)

processor=`uname -m`
if [[ ${processor} == *"x86_64"* ]]; then
  clang++ $1 -std=c++20 -g -O3 \
    -Wall -Wextra -Werror -pedantic -Wno-deprecated-volatile \
    -isystem ${googleBenchmark}/include \
    -isystem ${lptInc} \
    -mavx2 -mllvm -x86-asm-syntax=intel \
    -S -o - | llvm-mca-14 -mcpu=btver2 -timeline
fi

if [[ ${processor} == *"aarch64"* ]]; then
  clang++ $1 -std=c++20 -g -O3 \
    -Wall -Wextra -Werror -pedantic -Wno-deprecated-volatile \
    -isystem ${googleBenchmark}/include \
    -isystem ${lptInc} \
    -mllvm \
    -S -o - | llvm-mca-14  -timeline
fi

