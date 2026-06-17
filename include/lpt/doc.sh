#!/bin/bash 
set -x

SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)
/usr/bin/grep -r '\\brief' ${SCRIPT_DIR}/*

