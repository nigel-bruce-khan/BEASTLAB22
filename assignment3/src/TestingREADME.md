//*******To run files********//
c++ -std=c++17 -Wall -Wextra -O3 -march=native -g -DASSIGNMENT_SOLUTION -fopenmp -o testfile assignment3_part_1_template.c

//*****input three numbers instead of maximumDatasetSize, totalNumberProcessedPoints, and Stride*******//
OMP_NUM_PLACES=thread OMP_PROC_BIND=spread ./check maximumDatasetSize totalNumberProcessedPoints Stride
