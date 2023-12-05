#!/bin/bash
set -e
set -x

#
# LLVM MCA wrapper
# Usage: ./run_mca file.cpp
# 

googleBenchmark=${HOME}/work/benchmark
lptInc=../../include

target=$(basename -s \.cpp $1)

#TODO: -mavx2 
clang++ $1 -std=c++20 -g -O3 \
  -Wall -Wextra -Werror -pedantic -Wno-deprecated-volatile \
  -isystem ${googleBenchmark}/include \
  -isystem ${lptInc} \
  -mavx2 -mllvm -x86-asm-syntax=intel \
  -S -o - | llvm-mca-14 -mcpu=btver2 -timeline

