#!/bin/bash

make -B list


#echo "increasing size "
#for node in 1 2 3 4
#do
    echo $size
    echo "0 0"
    OMP_NUM_THREADS=1 OMP_PLACES=cores OMP_PROC_BIND=true numactl --cpubind=0 --membind=0 ./assignment3_part_2 16777216 50000 
    echo "0 1"
    OMP_NUM_THREADS=1 OMP_PLACES=cores OMP_PROC_BIND=true numactl --cpubind=0 --membind=1 ./assignment3_part_2 16777216 50000 

#done