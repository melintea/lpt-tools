#!/bin/bash
set -e
set -x

#
# Verify with relacy.
# Usage: ./run file.cpp
#

if [ $# -ne 1 ]; then
  echo "Usage: `basename $0` <cpp-file>"
  exit 1
fi

compiler=clang++
relacyHome=${HOME}/work/relacy

lptInc=../../../include

target=$(basename -s \.cpp $1)

${compiler} --version

g++ -MD -MF ${target}.o.d -MP -MT ${target}.o -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 -c -o ${target}.o $1
g++ -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 ${target}.o -o ${target}

#${compiler} $1 -std=c++20 -g -O3 \
#  -isystem ${lptInc} -isystem ${relacyHome} \
#  -o ${target} 
#

./${target} 

rm ./${target} ./${target}.o*
