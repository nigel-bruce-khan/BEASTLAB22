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

void triad(unsigned long datasetSize, unsigned long repetitions, long numTeams, long numThreads) {

    volatile auto *a = new double[datasetSize];
    auto *b = new double[datasetSize];
    auto *c = new double[datasetSize];
    auto *d = new double[datasetSize];

    // Device Ids CPU = 1 , GPU = 0 
    int targetDeviceId = 1;

    /*
    *TODO: Part 1.1 - Check by below code block to see if you can offload to GPU properly
    */


	#pragma omp target map(tofrom: targetDeviceId, numTeams, numThreads)
	{
	  targetDeviceId = omp_is_initial_device();
	}

	/*
	*TODO: Part 1.3: Variant 1 (CPU to GPU copy after CPU initialization ) is done by the for loop below. Implement and Test Variant 2 
	*/
	for (unsigned long i = 0; i < datasetSize; i++) {
		a[i] = b[i] = c[i] = d[i] = i;
	}

	auto start = std::chrono::high_resolution_clock::now();

	//to parallelize on cpu the access to each gpu
    #pragma omp parallel for num_threads(omp_get_num_devices())
	for (int dev = 0; dev < omp_get_num_devices(); dev++){
	
		if(dev == 0){
			//GPU offloading o f the computation loops 
			#pragma omp target device(0) nowait map(tofrom: a[0:datasetSize/2]) map(to: b[0:datasetSize/2], c[0:datasetSize/2], d[0:datasetSize/2]) 
			#pragma omp teams num_teams(numTeams)
			for (unsigned long j = 0; j < repetitions; ++j) {
				
				/*
				*TODO: Part 1.4: Add necessary scheduling clause to the pragma below. 
				*             (You can play and test with different scheduling schemes and chunk sizes to better see the effects)
				*/
				/*
				*TODO: Part 1.2: Add necessary clause to the pragma below to utilise all threads and teams to full extend
				*             (You can play and test with different scheduling schemes and chunk sizes to better see the effects)
				*/
				#pragma omp distribute parallel for num_threads(numThreads) schedule(static, 16)
				for (unsigned long i = 0; i < datasetSize/2; ++i) {
					a[i] = b[i] + c[i] * d[i];
				}
			}
		}
		
		else if(dev == 1){
			#pragma omp target device(1) nowait map(tofrom: a[datasetSize/2:datasetSize/2]) map(to: b[datasetSize/2:datasetSize/2], c[datasetSize/2:datasetSize/2], d[datasetSize/2:datasetSize/2]) 
			#pragma omp teams num_teams(numTeams)
			for (unsigned long j = 0; j < repetitions; ++j) {
				
				/*
				*TODO: Part 1.4: Add necessary scheduling clause to the pragma below. 
				*             (You can play and test with different scheduling schemes and chunk sizes to better see the effects)
				*/
				/*
				*TODO: Part 1.2: Add necessary clause to the pragma below to utilise all threads and teams to full extend
				*             (You can play and test with different scheduling schemes and chunk sizes to better see the effects)
				*/
				#pragma omp distribute parallel for num_threads(numThreads) schedule(static, 16)
				for (unsigned long i = datasetSize/2; i < datasetSize; ++i) {
					a[i] = b[i] + c[i] * d[i];
				}
			}
		}
	}

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration<double>(stop - start).count();

    double checksum = calculateChecksum(datasetSize, a);

    delete[] a;
    delete[] b;
    delete[] c;
    delete[] d;

    double mflops = calculateMegaFlopRate(datasetSize, repetitions, duration);
    printf("| %10ld | %8ld | %8ld | %8.2f | %8ld | %4d | %.4e |\n", datasetSize, numTeams, numThreads, mflops, repetitions, targetDeviceId, checksum);
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
    long numThreads = 1024;
    long numTeams = 100;
    if(argc >= 4 )
        numTeams = strtol(argv[3], &pEnd, 10);
    if(argc == 5 )
        numThreads = strtol(argv[4], &pEnd, 10);

    fprintf(
            stderr, "Maximum dataset size = %ld, total number of processed points = %ld. Performance in MFLOPS.\n",
            maximumDatasetSize, totalNumberProcessedPoints
    );


    printf("| %10s | %8s | %8s | %8s | %8s | %4s | %10s |\n", "Data size", "Teams", "Threads", "MFLOPS", "Cycles", "GPU", "Checksum");

    long datasetSize = 64;
    while (datasetSize <= maximumDatasetSize) {

        // Keep the total number of processed points constant by 
        // adjusting the number of repetitions according to data set size
        long cycles = std::clamp(totalNumberProcessedPoints / datasetSize, 8l, 65536l);

        // Run benchmark
        triad(datasetSize, cycles, numTeams, numThreads);

        datasetSize *= 2;
    }

    return 0;
}

