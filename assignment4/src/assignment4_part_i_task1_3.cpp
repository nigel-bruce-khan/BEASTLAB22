#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <chrono>
#include <algorithm>

#include <omp.h>

double calculateMegaFlopRate(long size, long repetitions, double duration) {
    return 2.0 * (double) size * (double) repetitions * 1.0e-6 / duration;
}

double calculateChecksum(long datasetSize, const volatile double* vector) {
    double checksum = 0;
    for (int i = 0; i < datasetSize; i++) {
        checksum += vector[i];
    }
    return checksum;
}

void triad(unsigned long datasetSize, unsigned long repetitions, long numteams, int scheduler, long threads, int transfer) {

    volatile auto *a = new double[datasetSize];
    auto *b = new double[datasetSize];
    auto *c = new double[datasetSize];
    auto *d = new double[datasetSize];

    // Device Ids CPU = 1 , GPU = 0 
    int targetDeviceId = 1;
    int chunk = datasetSize/numteams;
    int schedule = scheduler;
    

    /*
    *TODO: Part 1.1 - Check by below code block to see if you can offload to GPU properly
    */

    #pragma omp target map(tofrom: targetDeviceId, threads)
    {
      targetDeviceId = omp_is_initial_device(); /* returns true if the current task is executing
                                                 on the host device; otherwise, it returns false.*/
    
    }                               
    //Variant 1 (CPU to GPU copy after CPU initialization with scheduler 1 )
    if (transfer == 1){
        //initialize data on cpu and start timer
        auto start = std::chrono::high_resolution_clock::now(); //starts timer at CPU
        for (unsigned long i = 0; i < datasetSize; i++) {
            a[i] = b[i] = c[i] = d[i] = i;
        }

        //GPU offloading o f the computation loops : a is moved across host/devices, b,c,d remain in target device. 
            #pragma omp target map(tofrom: a[0:datasetSize]) map(to: b[0:datasetSize], c[0:datasetSize], d[0:datasetSize],threads, numteams)
                //numTeams is firstprivate,since is not in any map construct and its a scalar.  
                #pragma omp teams num_teams(numteams) 
                //distribute repetitions across teams 
                
                for (unsigned long j = 0; j < repetitions; ++j) { 
                    #pragma omp distribute parallel for num_threads(threads) 
                        for (unsigned long i = 0; i < datasetSize; ++i) {
                             a[i] = b[i] + c[i] * d[i];
                        }
                }
                
        auto stop = std::chrono::high_resolution_clock::now(); //stops timer at CPU
        auto duration = std::chrono::duration<double>(stop - start).count();
        double mflops = calculateMegaFlopRate(datasetSize, repetitions, duration);
        double checksum = calculateChecksum(datasetSize, a);
        printf("| %10ld | %8ld | %8ld | %8.2f | %8ld | %4d | %.4e |\n", datasetSize, numteams, numteams*threads, duration, repetitions, targetDeviceId, checksum);
    
    }
    //Variant 2 (GPU initialization with scheduler 1)
    else {
        auto start = std::chrono::high_resolution_clock::now(); //starts timer at CPU

            //the alloc map-type indicates that the corresponding memory for the array section on the accelerator 
            //is not initialized with any value, only storage is created
            #pragma omp target enter data map(alloc: a[0:datasetSize], b[0:datasetSize], c[0:datasetSize], d[0:datasetSize]) 
                #pragma omp target //the first target region which initializes the arrays           
                    for (unsigned long i = 0; i < datasetSize; i++) {
                        a[i] = b[i] = c[i] = d[i] = i;          
                     }
                
                
        //auto start = std::chrono::high_resolution_clock::now(); //starts timer at CPU

                //a, b,c,d remain in target device. 
                #pragma omp target map(threads, numteams)
                    #pragma omp teams num_teams(numteams) 
                    //distribute repetitions across teams 
                    
                    for (unsigned long j = 0; j < repetitions; ++j) { 
                        #pragma omp distribute parallel for num_threads(threads) 
                            for (unsigned long i = 0; i < datasetSize; ++i) {
                                 a[i] = b[i] + c[i] * d[i];
                            }
                    }        
            #pragma omp target exit data map(from: a[0:datasetSize]) map(delete: b[0:datasetSize], c[0:datasetSize], d[0:datasetSize])

        
        auto stop = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double>(stop - start).count();
        double mflops = calculateMegaFlopRate(datasetSize, repetitions, duration);
        double checksum = calculateChecksum(datasetSize, a);
        printf("| %10ld | %8ld | %8ld | %8.2f | %8ld | %4d | %.4e |\n", datasetSize, numteams, numteams*threads, duration, repetitions, targetDeviceId, checksum);


    }   

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;
    
    
}

int main(int argc, char *argv[]) {

   if (argc < 3) {
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
    /*
    * TODO: Part 1.5: You can adjust thread and team numbers by passing argv[3] and argv[4] to program
    */ 
    
    long numThreads = 32; //Max # of threads that can execute concurrently in a single parallel region.
    long numTeams = 15;
    int schedule = 1;
    int transfer = 0;
    

    if(argc >= 4 )
        //numTeams = strtol(argv[3], &pEnd, 10);
       transfer = strtol(argv[3], &pEnd, 10);

    if(argc == 5 )
        numThreads = strtol(argv[4], &pEnd, 10);

    fprintf(
            stderr, "Maximum dataset size = %ld, total number of processed points = %ld. Performance in MFLOPS.\n",
            maximumDatasetSize, totalNumberProcessedPoints
    );


    printf("| %10s | %8s | %8s | %8s | %8s | %4s | %10s |\n", "Data size", "Teams", "Threads", "duration", "Cycles", "GPU", "Checksum");

    long datasetSize = 64;
    while (datasetSize <= maximumDatasetSize) {

        // Keep the total number of processed points constant by 
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(totalNumberProcessedPoints / datasetSize, 8l, 65536l);


        // Run benchmark
        triad(datasetSize, cycles, numTeams, schedule, numThreads, transfer);

        datasetSize *= 2;
    }

    return 0;
}