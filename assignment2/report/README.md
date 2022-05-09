# Assignment 2 Report
Group: TODO

# 1a)

#### Can we safely permute the loops?
Yes for small matrices. For big matrices the performance of the implementation is different, because cache
misses are getting dominant for some of the implementations. The fastest implementation is kij, we iterate along
row of A and row of C (least cache misses). The slowest implementation is jki, we iterate along a column of A
(to which we write and read) and a column of C. The implementation ijk is in between, we iterate along a row of
B and column of C, while we write to the same location of A.

#### Explain which of the loops (i, j, k) is parallelizable?
It's possible to parallelize whichever of the outer loops (i or j), since they don't have data dependencies and two
distinct threads will not write to the same memory location. However, with the loops properly nested, outer loop parallelization (i) is usually better, since parallelizing the middle loop (j) increases the overhead and this severely affects the performance. 

When trying to paralleize the third loop (k), data races might occur.
To avoid this, atomic operations are necessary or a `#pragma omp parallel for reduction`.

# 1b)

#### Explain alternative parallelization schemes:

##### - Nested 
OpenMP uses a fork-join model of parallel execution. When a thread encounters a parallel construct, the thread creates a team composed of itself and some additional (possibly zero) number of threads (slave). The encountering thread becomes the master of the new team. When all team members have arrived at the barrier, the threads can leave the barrier. The master thread continues execution of user code beyond the end of the parallel construct, while the slave threads wait to be summoned to join other teams.

OpenMP parallel regions can be nested inside each other. If nested parallelism is **disabled**, then the new team created by a thread encountering a parallel construct inside a parallel region consists only of the encountering thread. If nested parallelism is **enabled**, then the new team may consist of more than one thread. Nested parallelism can be enabled or disabled by setting the **OMP_NESTED ** environment variable or calling **omp_set_nested()** function

The OpenMP runtime library maintains a pool of threads that can be used as slave threads in parallel regions. When a thread encounters a parallel construct and needs to create a team of more than one thread, the thread will check the pool and grab idle threads from the pool, making them slave threads of the team. The master thread might get fewer slave threads than it needs if there is not a sufficient number of the idle threads in the pool. When the team finishes executing the parallel region, the slave threads return to the pool.

##### - pragma omp parallel for collapse(2)
Here the nested looops ij are combined to make a single loop with N*N iterations. Useful to automatically assign iterations to threads


     #pragma omp parallel for collapse(2) schedule(...)
   
      for (i=0; i< N; i++) 
   
        for (j=0; j< N; j++) 
`

###### - Using pragma omp parallel for on each loop

If '#pragma omp parallel for' is used in only one loop, then only its iterations will be distributed along the threads, as seen in the example below, where only j is distributed across the threads (thread 0 gets i = 0, thread 1 gets i = 1...), and the same goes to i when the parallel for clause precedes it: 





By enabling worksharing in loops i and j, nested parallelism is disabled by default on OpenMP, and the second pragma is ignored at runtime: a thread enters the _inner parallel region_, a team of only one thread is created, and each inner loop is processed by a team of one thread. The end result will look, in essence, identical to what we would get without the second pragma — but there is just more overhead in the inner loop:
` 
    
        #pragma omp parallel for
        for (int i = 0; i < 3; ++i) {
            #pragma omp parallel for
            for (int j = 0; j < 6; ++j) {
                c(i, j);
            }
        }
 
On the other hand, if we tried to enable “nested parallelism”, things would get much worse. The inner parallel region would create more threads, and overall we would have more threads competing for the resources of limited CPU cores — not what we want in a performance-critical application. 

