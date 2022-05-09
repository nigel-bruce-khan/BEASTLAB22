CXX=c++
CFLAGS=-Wall -Wextra -Ofast -march=native -std=c++17 -fopenmp -fstrict-aliasing

all: template_task_a template_task_c assignment2 assignment2_sol_d

template_task_a:
	$(CXX) $(CFLAGS) -o template_task_a template_task_a.cpp

template_task_c:
	$(CXX) $(CFLAGS) -o template_task_c template_task_c.cpp

assignment2:
	$(CXX) $(CFLAGS) -o assignment2 assignment2.cpp

clean:
	rm -f template_task_a template_task_c assignment2

