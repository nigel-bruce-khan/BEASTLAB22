#!/bin/bash

make -B template_task_g

stdbuf --output=L ./template_task_g | tee ./perf_data.txt

python3 ../util/generate_chart.py
