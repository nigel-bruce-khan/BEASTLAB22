#include <chrono>
#include <ctime>
#include <unistd.h>
#include <iostream>
#include <cstring>
#include <sstream>
#include <math.h>
#include <omp.h>
#include<bits/stdc++.h> 


inline double get_duration(std::chrono::time_point<std::chrono::high_resolution_clock> t0, 
    std::chrono::time_point<std::chrono::high_resolution_clock> t1) {
    using dsec = std::chrono::duration<double>;
    
    return std::chrono::duration_cast<dsec>(t1 - t0).count();
}

struct entry {
    double v;
    struct entry* next;
};

// Initializes array of struct entry with random values.
// Note: next is not set!
bool init_AOS(struct entry* A, int N) {


    for(int i = 0; i < N; i++) {
        A[i].v = N;
    }
    for(int i = 0; i < N; i++){
        if(A[i].v == 0)
            return false;
    }
    return true;
}

// Initializes
bool init_llist(struct entry* A, int k, int N) {


    for(int i = 0; i < N; i++) {
        A[i].v = N;
    }

    for(int i = 0; i < N; i++){
        if(A[i].v == 0)
            return false;
    }

    for (int i = 0; i < N; i++) {
        A[i].next = 
    /*
    * TODO@Students: Q1a) Set A.next of each element to ensure same traversal pattern as sum1 
    */
    }

    return true;
}

double sum1(int64_t N, int k, int REP, double *sum) {

    struct entry* A ;
    /*
    * TODO@Students: Q1a) Allocate data structures
    */

    if(!init_AOS(A, N))
        std::cerr << "error in init_AOS" << std::endl;
    *sum = 0;

    auto t0 = std::chrono::high_resolution_clock::now () ;
    /*
    * TODO@Students: Q1a) Add your parallel solution for sum1 computation
    */
    auto t1 = std::chrono::high_resolution_clock::now();

    /*
    * TODO@Students: Q1a) Clean up data structures
    */
    // return accesses per second
    return (double)REP*(double)N / get_duration(t0, t1);
 
}


double sum2(int64_t N, int k, int REP, double *sum) {
    struct entry* A ;
    /*
    * TODO@Students: Q1a) Allocate data structures
    */

    if(!init_llist(A, k, N))
        std::cerr << "error in init_llist" << std::endl;

    *sum = 0.0;

    auto t0 = std::chrono::high_resolution_clock::now();
    /*
    * TODO@Students: Q1a) Add your parallel solution for sum2 computation
    */
    auto t1 = std::chrono::high_resolution_clock::now () ;
    /*
    * TODO@Students: Q1a) Clean up data structures
    */
    return (double)REP * (double)N / get_duration(t0, t1);
}


int main(int argc, char **argv) {

    if (argc != 3) {
        printf("The two parameters maximum dataset size and total number of processed points need to be provided.\n");
        exit(1);
    }

    char *pEnd;
    int64_t maximumDatasetSize = strtol(argv[1], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the first number.");
        exit(2);
    }
    int64_t totalNumberProcessedPoints = strtol(argv[2], &pEnd, 10);
    if (errno == ERANGE) {
        printf("Problem with the second number.");
        exit(3);
    }

    fprintf(
            stderr, "Maximum dataset size = %ld, total number of processed points = %ld. Performance in MFLOPS.\n",
            maximumDatasetSize, totalNumberProcessedPoints
    );

    printf("| %12s | %12s | %12s | %12s | %12s |\n", "Sum Type", "Data size", "Access Rate", "Cycles", "Checksum");
    /*                                                                                     /\
                                                                                          /  \
                                                                                         |    |
                                                                                       --:'''':--
                                                                                         :'_' :
                                                                                         _:"":\___
                                                                          ' '      ____.' :::     '._  */
    // static const double goldenRatio = 32951280099.0/20365011074.0; // . *=====<<=)           \    :
    /*                                                                    .  '      '-'-'\_      /'._.'
                                                                                           \====:_ ""
                                                                                          .'     \\
                                                                                         :       :
                                                                                        /   :    \
                                                                                       :   .      '.
                                                                                       :  : :      :
                                                                                       :__:-:__.;--'
                                                                                      '-'   '-'       */
    int64_t datasetSize = 1024; // =2^10
    int k = 0;
    double sum1_access_rate=0, sum2_access_rate=0 ;
    double sum_1=0, sum_2=0;
    while (datasetSize <= maximumDatasetSize) 
    {
        int64_t cycles = std::clamp(totalNumberProcessedPoints / datasetSize, 1l, 8l);

        sum1_access_rate = sum1(datasetSize, 1, cycles, &sum_1);
        printf( "| %12s | %12ld | %12.2f | %12ld | %12.1f |  \n", "Sum1 k=1",datasetSize, sum1_access_rate, cycles, sum_1);
        sum2_access_rate = sum2(datasetSize, 1, cycles, &sum_2);
        printf("| %12s | %12ld | %12.2f | %12ld | %12.1f |  \n", "Sum2 k=1",datasetSize, sum2_access_rate, cycles, sum_2 );
        
        /*
        * TODO@Students: Q1b) make sure N and k are coprime and set the k so that N/k is close to golden ratio - for pseudo-randomness
        */
        k = ;

        sum1_access_rate = sum1(datasetSize, k, cycles, &sum_1);
        printf("| %12s | %12ld | %12.2f | %12ld | %12.1f | \n", "Sum1 k=gold", datasetSize, sum1_access_rate, cycles, sum_1);
        sum2_access_rate = sum2(datasetSize, k, cycles, &sum_2);
        printf("| %12s | %12ld | %12.2f | %12ld | %12.1f | \n", "Sum2 k=gold", datasetSize, sum2_access_rate, cycles, sum_2 );
        datasetSize *= 2;
    }
}
