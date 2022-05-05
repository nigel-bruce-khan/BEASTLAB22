#!/bin/bash

make -B assignment2

stdbuf --output=L ./assignment2 | tee ./perf_data.txt

python3 ../util/generate_chart.py
