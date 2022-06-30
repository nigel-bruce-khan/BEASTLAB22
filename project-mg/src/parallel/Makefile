#CXXFLAGS=-std=c++17 -O3 -march=native -mavx -g
CXXFLAGS = -Wall -Wextra -O3 -fno-inline -faligned-new -march=native -mavx -g -fopenmp 
OBJS = main.o

all: multigrid

multigrid: $(OBJS)
	g++ $(OBJS) $(CXXFLAGS) -o multigrid

clean:
	rm -f *~ *.o *.vtk multigrid

