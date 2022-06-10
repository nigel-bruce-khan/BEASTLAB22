CXXFLAGS=-O2

OBJS = main.o

all: multigrid

multigrid: $(OBJS)
	g++ $(OBJS) -o multigrid

clean:
	rm -f *~ *.o *.vtk multigrid

