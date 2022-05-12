#!/bin/bash

make -B assignment2


echo "ikj 100 close static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ikj 100 static
done

echo "ikj 100 spread static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ikj 100 static
done

echo "ikj 1900 close static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ikj 1900 static
done

echo "ikj 1900 spread static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ikj 1900 static
done

echo "ikj 100 close dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ikj 100 dynamic
done

echo "ikj 100 spread dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ikj 100 dynamic
done

echo "ikj 1900 close dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ikj 1900 dynamic
done

echo "ikj 1900 spread dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ikj 1900 dynamic
done

echo "ijk 100 close static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ijk 100 static
done

echo "ijk 100 spread static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ijk 100 static
done

echo "ijk 1900 close static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ijk 1900 static
done

echo "ijk 1900 spread static"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ijk 1900 static
done

echo "ijk 100 close dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ijk 100 dynamic
done

echo "ijk 100 spread dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ijk 100 dynamic
done

echo "ijk 1900 close dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=close ./assignment2 ijk 1900 dynamic
done

echo "ijk 1900 spread dynamic"
for thread in 1 2 4 8 16 32 48 64 96 144 256 
do
    OMP_NUM_THREADS=$thread OMP_PROC_BIND=spread ./assignment2 ijk 1900 dynamic
done