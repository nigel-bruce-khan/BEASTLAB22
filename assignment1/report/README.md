# Assignment 1 Report
Group: TODO

**TODO@Students: Write your report in this folder using Markdown.**

Assignment1:

**A)**  The size  of vectors and number of iterations are retrieved from the user input, in this case the variables N and REP. These values need to be higher than the predefined variables datasetsize (32) and cycles, in order to pass the comparison clause from the while loop. This statement requires that the vectors to be large enough to store the all the values from the existing dataset, and also to perform at least 1 cycle in the triad routine. When entering the while loop (caller) on line 116, we are calling the function “triad” (the callee) whose parameters are const long N,  const long REP, and int *numThreads. It’s in this function where the vectors are constructed, with size N and number of iterations REP. With each loop iteration from line 116, we are modifying  the variables datasetSize and cycles, which are then passed as argument to the callee function, meaning that with each call the size and number or iterations are being changed. 

One can observe that in the formula (from triad()):
a[j] = b[j] + c[j] * d[j] 
There are two arithmetic operations (one multiplication	and one addition). Moreover, this instruction is executed inside a nested loop: the inner loop runs N times, and the outer is executed REP times. Therefore the total number of floating point operations  (2) are performed N *REP times. Recall that triad is being called with parameters datasetSize and cycles, hence the number of floating point operations computed into the loop , denoted by m_flop is equal to:
2* datasetSize*cycles. Since we are counting per million, we multiply by 1*10^-6 to get the result in millions of operations. 

B) As mentioned above, triad takes as parameters two const long datatypes, N and REP, as well as a pointer to an integer which stores the address of the number of threads. 
N = Determines the size of the vectors
REP = how many times to recompute vector A.



Triad completes by returning a variable of type double named time_spent, indicating how much time was spent on the simulations. 

C) we use omp parallel to create a parallel region where threads can be spanned. Using the definition from IBM, “The omp parallel directive explicitly instructs the compiler to parallelize the chosen block of code”

D) aligned_loc() is a function introduced by the new C11 standard. It allows to allocate memory spaces with given alignments greater than those admitted by malloc. This function has the following structure: void *aligned_alloc(size_t __alignment, size_t __size)

 
The requirements to use it are:
- 1) the size (second argument) requested must be an integral multiple of the alignment (first argument) and 
2) the value of alignment should be a valid alignment supported by the implementation.

 Failure to meet either of them results in undefined behaviour.

In the code, the space allocated for each vector is computed as N*sizeof(double). Since the minimum value that N can take is 32 (the first iteration of the while loop on line 116 starts with datasetSize set to 32, which is passed as an argument to triad), and sizeof(double) equals 64, this results in spaces of size 2048x bytes (x is an integer multiplication factor).Because of condition 1)  4096 is an integral multiple of the assigned size. For condition 2), one can check on each of the ….

E) As discussed in the lecture, when we are trying to optimise for NUMA architecture it is better to parallelise the initialisation of the vectors in the same way that they are later accessed, meaning that the vectors are distributed across locality domains with the first-touch allocation policy. By doing it this way, we reduce the latency and improve the performance regarding memory fetching operations. The static scheduler is used to guarantee that the working load is distributed uniformly across threads on a round-robin fashion, helping to maximise the memory locality, while also offering a deterministic behaviour. 

F) …..

G ) When using a parallel region, OpenMP automatically wait for all threads to finish before execution continues (implicit barrier). Furthermore, there is also a synchronization point after each “omp for” loop, implying that threads need to wait until each of them has finished their respective tasks. Since there are no dependencies in the loop and synchronization is not needed after the loop, we disable this by using nowait, which also reduce the idle time for each thread. Thus, the time_spent will consider only the execution time starting at the creation of the 
