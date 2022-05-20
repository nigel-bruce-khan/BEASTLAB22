ifeq ($(NODE),thx) # ThunderX systems
	#@module load cuda/11.1.1
	#module load llvm/11.0.0_nvptx_offloading
	CXX = clang++
	CXX_FLAGS = -std=c++17 -Wall -Wextra -Ofast -g -fopenmp -fopenmp-targets=nvptx64 -Xopenmp-target=nvptx64 -march=sm_70
else # Rome systems
	CXX = /opt/rocm/llvm/bin/clang++
	CXX_FLAGS = -std=c++17 -Wall -Wextra -Ofast -g -fopenmp -target x86_64-pc-linux-gnu -fopenmp-targets=amdgcn-amd-amdhsa -Xopenmp-target=amdgcn-amd-amdhsa -march=gfx906
endif

all: assignment4_i assignment4_ii

assignment4_i: assignment4_part_i.cpp
	${CXX} ${CXX_FLAGS} assignment4_part_i.cpp -o assignment4_i

assignment4_ii: assignment4_part_ii.cpp
	${CXX} ${CXX_FLAGS} assignment4_part_ii.cpp -o assignment4_ii


# For this assignment we don't have CI tests.

clean:
	rm -f assignment4_i assignment4_ii  util/part2/perf_data.txt util/part2/perf_data.pdf util/part2/perf_data.svg util/part1/perf_data.txt util/part1/perf_data.pdf util/part1/perf_data.svg