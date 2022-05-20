#!/bin/bash

# TODO@Students: Part I Q2b) Adjust for NUMA effects, first touch, and thread pinning

if [ -z $1 ]
then
  echo "Usage: test_script.sh <p1|p2|all>"
  echo "Mode: please specify whether to run part 1 or part 2 on gpu."
  exit 255
fi

# This statement runs your program in line buffering mode, duplicating stdout to stdout and perf_data.txt.
# This makes it suitable for processing using CI.
case "$1" in
  "p1")
    OMP_TARGET_OFFLOAD=mandatory stdbuf --output=L ./assignment4_i 134217728 268435456 | tee gpu_i.txt
    ;;
  "p2")
    OMP_TARGET_OFFLOAD=mandatory stdbuf --output=L ./assignment4_ii 32 256 | tee gpu_ii.txt
    ;;
  "all")
    bash "$0" p1 || exit 255
    bash "$0" p2 || exit 255
    ;;
  *)
    echo "Unknown code version: $1"
esac

	
