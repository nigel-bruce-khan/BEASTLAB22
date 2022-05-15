#!/bin/bash

OMP_NUM_PLACES=threads ./assignment3_part_1 3355 67 1 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 2 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 4 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 8 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 16 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 32 >> results.txt

OMP_NUM_PLACES=threads ./assignment3_part_1 33554432 67108864 64 >> results.txt

