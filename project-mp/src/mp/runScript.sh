#!/bin/sh

make clean
make
make run
g++ validate.cpp
./a.out
