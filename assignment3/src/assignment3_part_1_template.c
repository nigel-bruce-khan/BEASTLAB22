#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <chrono>
#include <algorithm>

#include <omp.h>
#include <cassert>

double get_curr_time(){
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return t.tv_sec + t.tv_nsec * 1e-9;
}


double calculateMegaFlopRate(long size, long repetitions, double duration) {
    /*
     * TODO@Students: Q1a) Calculate MegaFLOP rate
     */
     double m_flop;
     double performance_a;
     m_flop = 2.0 * (double)size * (double)repetitions * 1.0e-6;
     return performance_a = m_flop / duration;

}

double calculateChecksum(long datasetSize, const volatile double* vector, int stride) {
    double checksum = 0;
    for (int i = 0; i < datasetSize; i+=stride) {
        checksum += vector[i];
    }
    return checksum;
}

void triad(long datasetSize, long repetitions, long numThreads, int stride) {

    /*
    * TODO@Students: Q1a) Add your parallel solution for triad benchmark implementation from assignment 1 
    * TODO@Students: Q1a) Increment the interator of the inner for loop - where you do triad computations - in step sizes of stride
    */
    
    double time_spent = 0.0;
    double begin, end;
    
//****let N = dataSize/stride to give the number of triad computations done over the arrays
	long N = datasetSize/stride;


// TASK 1.c
#pragma omp parallel
{
	numThreads=omp_get_num_threads();
}

// TASK 1.d
    double* a = (double*) aligned_alloc (4096, datasetSize * sizeof(double));
    double* b = (double*) aligned_alloc (4096, datasetSize * sizeof(double));
    double* c = (double*) aligned_alloc (4096, datasetSize * sizeof(double));
    double* d = (double*) aligned_alloc (4096, datasetSize * sizeof(double));

// TASK 1.e
#pragma omp parallel for schedule(static)
    for (long j=0; j<N*stride; j++) {
	    a[j] = 0.0;
	    b[j] = 1.0;
	    c[j] = 2.0;
	    d[j] = 3.0;
    }

// TASK 1.f
#pragma omp parallel
{
    for (long i=0; i<repetitions; i++)
#pragma omp for schedule(static) nowait
        for (long j=0; j<N*stride; j+=stride)
            a[j] = b[j]+c[j]*d[j];
}

// TASK 1.g
    begin = get_curr_time();
#pragma omp parallel
{
    for (long i=0; i<repetitions; i++)
#pragma omp for schedule(static) nowait
        for (long j=0; j<N*stride; j+=stride)
            a[j] = b[j]+c[j]*d[j];
}
    end = get_curr_time();
    time_spent = end - begin;

// TASK 1.h
    double sum = calculateChecksum(datasetSize, a, stride);
    assert (abs(sum-N*7.0)<0.1);
    
    free(a); free(b); free(c); free(d);


    double mflops = calculateMegaFlopRate(N, repetitions, time_spent);
    printf("| %10ld | %8d | %8.2f | %8ld | %.4e |\n", datasetSize, stride, mflops, repetitions, sum);
}

int main(int argc, char *argv[]) {

	//changed number of arguments from 3 to 4 to include the STRIDE argument
    if (argc != 4) {
        printf("The two parameters maximum dataset size and total number of processed points need to be provided.\n");
        exit(1);
    }

    char *pEnd;
    long maximumDatasetSize = strtol(argv[1], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the first number.");
        exit(2);
    }
    long totalNumberProcessedPoints = strtol(argv[2], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the second number.");
        exit(3);
    }
    int stride = strtol(argv[3], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the third number.");
        exit(4);
    }

    fprintf(
            stderr, "Maximum dataset size = %ld, total number of processed points = %ld. Performance in MFLOPS.\n",
            maximumDatasetSize, totalNumberProcessedPoints
    );
    printf("| %10s | %8s | %8s | %8s | %10s |\n", "Data size", "Stride", "Access Rate", "Cycles", "Checksum");

    /*
    * TODO@Students: Q1b) Try all different stride values from the set {1, 2, 4, 8, 16, 32, 64} 
    */
    long  datasetSize = 64;
    while (datasetSize <= maximumDatasetSize) {
        // Keep the total number of processed points constant by adjusting the number of repetitions according to data
        // set size
        long cycles = std::clamp(totalNumberProcessedPoints / datasetSize, 8l, 65536l);
        long threads = omp_get_max_threads();
        triad(datasetSize, cycles, threads, stride);
            
        datasetSize *= 2;  
    }
    return 0;
}

