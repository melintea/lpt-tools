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

compiler=g++
relacyHome=${HOME}/work/relacy

lptInc=../../../include

target=$(basename -s \.cpp $1)

${compiler} --version

g++ -MD -MF ${target}.o.d -MP -MT ${target}.o -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 -c -o ${target}.o -g $1
g++ -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 ${target}.o -g -o ${target}

#${compiler} $1 -std=c++20 -g -O3 \
#  -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -I ${lptInc} \
#  -o ${target} 

./${target} 

rm ./${target} ./${target}.o*
