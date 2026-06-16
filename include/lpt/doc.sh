#!/bin/bash 
set -x

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
/usr/bin/grep '\\brief' ${SCRIPT_DIR}/*

