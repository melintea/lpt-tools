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

# https://github.com/ccotter/relacy
relacyHome=${HOME}/work/relacy-cotter

# https://github.com/dvyukov/relacy
#relacyHome=${HOME}/work/relacy-dvyukov

lptInc=../../../include

target=$(basename -s \.cpp $1)

${compiler} --version

#
# No -std=c++20 
#

#${compiler} -MD -MF ${target}.o.d -MP -MT ${target}.o -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 -c -o ${target}.o -g $1
#${compiler} -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -O1 ${target}.o -g -o ${target}

${compiler} $1 -g -O0 \
  -I ${relacyHome} -I ${relacyHome}/relacy/fakestd -I ${lptInc} \
  -o ${target} 

#./${target} 
cgdb ./${target} 

rm ./${target} ./${target}.o*
