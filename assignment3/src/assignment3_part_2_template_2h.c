#include <chrono>
#include <iostream>
#include <cstring>
#include <sstream>
#include <bits/stdc++.h>
#include <math.h>
#include <omp.h>


typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint;

inline double get_duration(TimePoint t0, TimePoint t1) {
    //using dsec = std::chrono::duration<double>;
    return std::chrono::duration_cast<std::chrono::nanoseconds>(t1 - t0).count();
}


struct entry {
    double v; //takes the value of i that will be summed up
    int64_t next; //stores index of next element
    int u; //auxiliar variable for veryfing access
};

/*
 * TODO@Students: Q2a) Verify that all elements are accessed
 */

void verify_access(int64_t N, struct entry* A){
    for(int64_t i = 0; i<N; i++){
        if (A[i].u !=1)
        {
            printf("Not all elements are accessed, A[%ld] is = %d \n", i, A[i].u);
            exit(1);
        }
        
    }
}
/*
 * TODO@Students: Q2a) Implement the list initialization
 */

void init(int64_t N, int64_t k, struct entry* A) {
    int64_t mask = N - 1; // N is power-of-2
    for (int64_t i = 0; i < N; ++i) {
        A[i].v = (double)i;
        A[i].next = (k * (i + 1)) & mask;
        //printf("A[%ld]   next =  %ld \n", i, A[i].next);
        A[i].u = 0; 
       
    }
}

// sum_indexcalc_time = sum_indexcalc(N, 1, cycles, &sum_calc, &dummy, )
double sum_indexcalc(int64_t N, int64_t k, int REP, double *psum, int64_t* pdummy) {
    double sum;
    int64_t mask = (N - 1); // N is power - of -2
    int64_t dummy = 0; //since we want the same accesses in both traversals. 

    //allocating memory for an object of size N*entry
    struct entry* A = (struct entry*) malloc(N*sizeof(struct entry));

    //initialize list
    init(N, k, A);
    
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < REP; ++r) {
        sum = 0.0;
        int64_t next = 0;
        for (int64_t i = 0; i < N; ++i) {
            sum += A[next].v;
            A[next].u = 1;
            dummy |= A[next].next;
            next = (k * (i + 1)) & mask; //(k*(i+1) mod N)th element
            //A[next].u = 1;
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();

    *psum = sum; *pdummy = dummy;

    verify_access(N, A);
    
    //get duration from timers, and divide it by the product of REPS and N
    //then multiply by 1.0e9 to get nanoseconds. 
    double average_time = get_duration(t0, t1)/(REP*N);

    //clean up data structures
    free(A);

    return average_time;
    }


double sum_indexload(int64_t N, int64_t k, int REP, double *psum, int64_t* pdummy) {
    double sum;
    int64_t mask = (N - 1); // N is power - of -2
    int64_t dummy = 0;

    /*
     * TODO@Students: Q2a) Allocate and initialize data structures
     */

    struct entry* A = (struct entry*) malloc(N*sizeof(struct entry));
    init(N, k, A);

    auto t0 = std::chrono::high_resolution_clock::now();
    for (int r = 0; r < 1; ++r) {
        sum = 0.0;
        int64_t next = 0;
        for (int64_t i = 0; i < N; ++i) {
            sum += A[next].v;
            A[next].u = 1;
            dummy |= (k * (i + 1)) & mask; //OR operation and assignment
            next = A[next].next; //gets the index of the next element to access from the previously accessed element.
            //A[next].u = 1;
            //printf("i =  %ld  next =  %ld \n", i, next);
                                
        }
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    
    *psum = sum; *pdummy = dummy;

    verify_access(N, A);
    
    /*
     * TODO@Students: Q2a) Clean-up data structures and
     *   return average time per element access in nanoseconds
     */
    double average_time = get_duration(t0, t1)/(REP*N);
    free(A);
    return average_time;

}


int main(int argc, char **argv) {

    if (argc != 3) {
        printf("The two parameters maximum dataset size and total number of processed points need to be provided.\n");
        exit(1);
    }

    char *pEnd;
    int64_t maximumDatasetSize = strtoll(argv[1], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the first number.");
        exit(2);
    }
    int64_t totalNumberProcessedPoints = strtoll(argv[2], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the second number.");
        exit(3);
    }
    int my_place = omp_get_place_num();
    int place_num_procs = omp_get_place_num_procs(my_place);
    printf("Place consists of %d processors and thread is at place= %d \n ", place_num_procs, my_place);
    
    fprintf(
            stderr, "Maximum dataset size = %ld, total number of processed points = %ld. Performance in MFLOPS.\n",
            maximumDatasetSize, totalNumberProcessedPoints
    );

    printf("| %12s | %12s |%12s | %12s | %12s | %12s |\n", "Sum Type", "K", "Data size", "Latency [ns]", "Cycles", "Checksum");
    
    /*                                                                                     /\
                                                                                          /  \
                                                                                         |    |
                                                                                       --:'''':--
                                                                                         :'_' :
                                                                                         _:"":\___
                                                                          ' '      ____.' :::     '._  */
    static const double goldenRatio = 32951280099.0/20365011074.0; //    . *=====<<=)           \    :
    /*                                                                    .  '      '-'-'\_      /'._.'
                                                                                           \====:_ ""
                                                                                          .'     \\
                                                                                         :       :
                                                                                        /   :    \
                                                                                       :   .      '.
                                                                                       :  : :      :
                                                                                       :__:-:__.;--'
                                                                                      '-'   '-'       */
    int64_t N = maximumDatasetSize; // =2^10 always a power of two
    int64_t k = 0;
    double Nn = 0.0;
    double sum_indexcalc_time=0, sum_indexload_time=0 ;
    double sum_calc=0, sum_load=0;
    int64_t cycles;
    int64_t dummy;
  

    while (N <= maximumDatasetSize) 
    {       
        cycles = std::clamp(totalNumberProcessedPoints / N, (int64_t)1, (int64_t)1000);

        //sum_indexcalc_time = sum_indexcalc(N, 1, cycles, &sum_calc, &dummy);
        //printf("| %12s |  1  | %12ld | %12.2f | %12ld | %12.1f |  \n", "sum_indexcalc k=1",N, sum_indexcalc_time, cycles, sum_calc);
        //sum_indexload_time = sum_indexload(N, 1, cycles, &sum_load, &dummy);
        //printf("| %12s |  1  | %12ld | %12.2f | %12ld | %12.1f |  \n", "sum_indexload k=1",N, sum_indexload_time, cycles, sum_load );

        /*
         * TODO@Students: Q2a) make sure N and k are coprime and set the k so that N/k is close to golden ratio - for pseudo-randomness
         */
         
    
        /*
        two integers a and b are coprime, 
        relatively prime or mutually prime if the only positive integer 
        that is a divisor of both of them is 1. Since N is a power of 2, we tested all numbers k
        that aren't multiples of 2 and whose ratio with N is close to the golden radio within
        a tolerance of 0.4. 
        
        
        */ 
       
        for (k = 3;;k++)
        {   Nn = (double)N;
            double error = abs(goldenRatio - ceil(Nn/k));
            
            if ((k % 2 != 0) && error < 0.4)
            {
                //printf("chosen k = %ld with N/K =%f\n", k, Nn/k);
                
                break;
            }
            if (k > 10000000000000)
            {
                printf("k was not found");
                exit(1);
            }

        }
     
        
        sum_indexcalc_time = sum_indexcalc(N, k, cycles, &sum_calc, &dummy);
        printf("| %12s | %12ld |%12ld | %12.2f | %12ld | %12.1f | \n", "sum_indexcalc k=gold", k, N, sum_indexcalc_time, cycles, sum_calc);
        sum_indexload_time = sum_indexload(N, k, cycles, &sum_load, &dummy);
        printf("| %12s | %12ld | %12ld | %12.2f | %12ld | %12.1f | \n", "sum_indexload k=gold", k, N, sum_indexload_time, cycles, sum_load );

        N *= 2;
    }
    return 0;
}
