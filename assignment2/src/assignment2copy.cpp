// put your working code in here to use the test_script.sh and CI

#include <chrono>
#include <math.h>
#include <cstdio>
#include <omp.h>
#include <string>
#include <cstring>



// use a 1d array to guarantee a dense and sequential memory layout
#define TWO_D_ACCESS(row, col, width) ((width) * (row) + (col))

//make sure we are calling the program with increasing number of threads



double mm(int N, int REP, double expected_result, char* argv[], char* sch[]) {

  //////////////////////////////////////////////////////////////////////////
  // ALLOCATION
  /////////////////////////////////////////////////////////////////////////

  double * a = (double *) malloc(N*N*sizeof(double));
  double * b = (double *) malloc(N*N*sizeof(double));
  double * c = (double *) malloc(N*N*sizeof(double));

  //////////////////////////////////////////////////////////////////////////
  // ALLOCATION
  /////////////////////////////////////////////////////////////////////////

  for(int i = 0; i < N*N; ++i){
    a[i] = 0;
    // initialize with something so the compiler/cpu cannot optimize 0*0=0
    b[i] = atan(i);
    c[i] = cos(i);
  }

  //////////////////////////////////////////////////////////////////////////
  // WARM UP
  /////////////////////////////////////////////////////////////////////////
  
  for( int r=0; r<REP; ++r ) {
    for( int i=0; i<N; i++ ) {
      for( int k=0; k<N; k++ ) {
        for( int j=0; j<N; j++ ) {
          a[TWO_D_ACCESS(i, j, N)] += b[TWO_D_ACCESS(i, k, N)] * c[TWO_D_ACCESS(k, j, N)];
        }
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////
  // MATRIX MULTIPLICATION KERNELS
  /////////////////////////////////////////////////////////////////////////
  
  //Using pragma omp parallel for on outer loop i
  auto t0 = std::chrono::high_resolution_clock::now();
  auto t1 = std::chrono::high_resolution_clock::now();

  if (strcmp(argv[0], "ikj") == 0 && strcmp(sch[0], "static") == 0) {
    t0 = std::chrono::high_resolution_clock::now();

    for( int r=0; r<REP; ++r ) {
      #pragma omp parallel for schedule(static)
      for( int i=0; i<N; i++ ) {
        for( int k=0; k<N; k++ ) {
          for( int j=0; j<N; j++ ) {
            a[TWO_D_ACCESS(i, j, N)] += b[TWO_D_ACCESS(i, k, N)] * c[TWO_D_ACCESS(k, j, N)];
          }
        }
      }
    }
    t1 = std::chrono::high_resolution_clock::now();
  }

  if (strcmp(argv[0], "ikj") == 0 && strcmp(sch[0], "dynamic") == 0) {
    t0 = std::chrono::high_resolution_clock::now();

    for( int r=0; r<REP; ++r ) {
      #pragma omp parallel for schedule(dynamic, 10)
      for( int i=0; i<N; i++ ) {
        for( int k=0; k<N; k++ ) {
          for( int j=0; j<N; j++ ) {
            a[TWO_D_ACCESS(i, j, N)] += b[TWO_D_ACCESS(i, k, N)] * c[TWO_D_ACCESS(k, j, N)];
          }
        }
      }
    }
    t1 = std::chrono::high_resolution_clock::now();
  }

  
// USING PRAGMA OMP PARALLEL FOR COLLAPSE(2) on loops i&k

  if (strcmp(argv[0], "ijk") == 0 && strcmp(sch[0], "static") == 0) {
    t0 = std::chrono::high_resolution_clock::now();

    for( int r=0; r<REP; ++r ) {
      #pragma omp parallel for collapse(2) schedule(static)
      for( int i=0; i<N; i++ ) {
        for( int j=0; j<N; j++ ) {
          for( int k=0; k<N; k++ ) {
            a[TWO_D_ACCESS(i, j, N)] += b[TWO_D_ACCESS(i, k, N)] * c[TWO_D_ACCESS(k, j, N)];
          }
        }
      }
    }
    t1 = std::chrono::high_resolution_clock::now();
  }

  if (strcmp(argv[0], "ijk") == 0 && strcmp(sch[0], "dynamic") == 0) {
    t0 = std::chrono::high_resolution_clock::now();

    for( int r=0; r<REP; ++r ) {
      #pragma omp parallel for collapse(2) schedule(dynamic, 10)
      for( int i=0; i<N; i++ ) {
        for( int j=0; j<N; j++ ) {
          for( int k=0; k<N; k++ ) {
            a[TWO_D_ACCESS(i, j, N)] += b[TWO_D_ACCESS(i, k, N)] * c[TWO_D_ACCESS(k, j, N)];
          }
        }
      }
    }
    t1 = std::chrono::high_resolution_clock::now();
  }
  









  // simple correctness check
  double array_sum = 0;
  for( int i=0; i<N*N; ++i ) {
    array_sum += a[i];
  }
  // verify expected result. accounting for possibly system dependent float rounding errors
  if(abs(array_sum - expected_result) > 0.1){
    printf("Wrong result for N=%4d. expected %.3f but got %.3f. Aborting...\n", N, expected_result, array_sum);
    exit(EXIT_FAILURE);
  }
  //printf("Solution(N=%4d): %.3f\n", N, array_sum);

  free((void *) a);
  free((void *) b);
  free((void *) c);
  using dsec = std::chrono::duration<double>;
  double dur = std::chrono::duration_cast<dsec>(t1-t0).count();
  printf("TIME: %d \n", dur);

  double mflop = 2.0*(double)N*(double)N*(double)N*(double)REP*1.0e-6;
  return mflop/dur;
}



int main(int argc, char* argv[]) {
  // ugly hardcoded stuff
  double mf;
  
  /*#pragma omp parallel
  {
    threads = omp_get_num_threads(); 
  }
  
  printf("THREAD NUMBER =%d \n", threads);*/


  //For task e we only use N= [100, 1900]
  
  if (strcmp(argv[2], "100") == 0) {
    mf = mm(100, 3200, 702147.857, &argv[1], &argv[3]);
    printf("MFLOPS(N=%4d): %.3f\n", 100, mf);
  }
  /*mf = mm(100, 3200, 702147.857);
  printf("MFLOPS(N=%4d): %.3f\n", 100, mf);

  mf = mm(200, 1600, 1211484.385);
  printf("MFLOPS(N=%4d): %.3f\n", 200, mf);

  mf = mm(300, 800, -212360.190);
  printf("MFLOPS(N=%4d): %.3f\n", 300, mf);

  mf = mm(400, 400, -257987.574);
  printf("MFLOPS(N=%4d): %.3f\n", 400, mf);

  mf = mm(500, 200, -115630.730);
  printf("MFLOPS(N=%4d): %.3f\n", 500, mf);

  mf = mm(600, 100, -92905.739);
  printf("MFLOPS(N=%4d): %.3f\n", 600, mf);

  mf = mm(700, 60, -49174.596);
  printf("MFLOPS(N=%4d): %.3f\n", 700, mf);

  mf = mm(800, 40, 102953.727);
  printf("MFLOPS(N=%4d): %.3f\n", 800, mf);

  mf = mm(900, 20, 55208.396);
  printf("MFLOPS(N=%4d): %.3f\n", 900, mf);

  mf = mm(1000, 10, -9100.239);
  printf("MFLOPS(N=%4d): %.3f\n", 1000, mf);

  mf = mm(1100, 5, 19076.488);
  printf("MFLOPS(N=%4d): %.3f\n", 1100, mf);

  mf = mm(1300, 2, -4334.610);
  printf("MFLOPS(N=%4d): %.3f\n", 1300, mf);

  mf = mm(1500, 1, 1062.209);
  printf("MFLOPS(N=%4d): %.3f\n", 1500, mf);

  mf = mm(1700, 1, -2689.469);
  printf("MFLOPS(N=%4d): %.3f\n", 1700, mf);

  mf = mm(1900, 1, 9179.534);
  printf("MFLOPS(N=%4d): %.3f\n", 1900, mf);*/

  if (strcmp(argv[2], "1900") == 0) {
    mf = mm(1900, 1, 9179.534, &argv[1], &argv[3]);
    printf("MFLOPS(N=%4d): %.3f\n", 1900, mf);
  }

 
  return 0;
}
