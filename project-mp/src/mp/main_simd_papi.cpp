#include<cstdio>
#include<cstdlib>
#include<string>
#include<cmath>
#include <iostream>
#include <omp.h>
#include <vector>
#include <algorithm>
#include <chrono>
#include "papi.h"

using namespace std;

struct MyMax {
  double max_cr;
  int idx;
  
};

#pragma omp declare reduction(maximo : struct MyMax : omp_out = omp_out.max_cr > omp_in.max_cr ? omp_out : omp_in)initializer(omp_priv={-1.0, -1})

double get_curr_time(){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec * 1e-9;
}

static inline int ind(const int i, const int j, const int mlen){
	return j+i*mlen;
}

void handle_error(int i){
  printf("Error while configuring PAPI");
  exit(EXIT_FAILURE);
}


template<typename T>
void print_buf(const string desc, const T *buf, const int len){
    printf(desc.c_str());
    printf("\n");
    for (int i=0; i<len; i++)
      //printf("%f, ",buf[i]);
      std::cout << buf[i] << ", "  ;
    printf("\n");
    printf("=========\n");
    printf("\n");
}

void precom_mu (double* ts, double* mu, int mlen, int sublen){

  double sum = 0;
  //#pragma omp parallel for schedule(static) num_threads(sublen) reduction(+ :sum)
  for (int i=0; i<sublen; i++){
    sum+=ts[i];
  }
  mu[0]=sum/sublen;

  //#pragma omp parallel for schedule(static) num_threads(sublen)
  for (int i=1; i<mlen; i++){
    mu[i] = mu[i-1] + ( ts[i+sublen-1]-ts[i-1] ) / sublen;
  }
  
  return;
}

void precom_norm(double* ts, double* norm, double* mu, int mlen, int sublen){

  double sum, tmp;
  #pragma omp parallel for schedule(static) num_threads(128) 
  for (int i=0; i<mlen; i++){
    sum = 0.0;
    tmp = 0.0;
    
    for (int j=0; j<sublen; j++){
      tmp = ts[i+j]-mu[i];
      sum+=tmp*tmp;
    }
    norm[i] = sqrt(1.0/sum);
  }

}

void precom_df(double* ts, double* df, int mlen, int sublen){

  df[0]=0;
  #pragma omp parallel for schedule(static) num_threads(128)
  for (int i=0; i<mlen-1; i++){
    df[i+1] = ( ts[i+sublen]-ts[i] ) * 0.5 ;
  }

  return;
}

void precom_dg(double* ts, double* mu, double* dg, int mlen, int sublen){

  dg[0] = 0;
  #pragma omp parallel for schedule(static) num_threads(128)
  for (int i=0; i<mlen-1; i++){
    dg[i+1] = ts[i+sublen] - mu[i+1] +  ts[i] - mu[i]; //TODO doublly check this
  }

  return;
}

int main(int argc, char* argv[]){

  // argument handling
  if(argc < 4){
    printf("Usage: exe <input file> <len> <sublen>\n");
    exit(0);
  }

  int len = atoi(argv[2]); 
  FILE* f = fopen(argv[1],"r");
  int sublen = atoi(argv[3]);
  int mlen = len - sublen + 1;

#ifdef DEB
	  printf("file: %s, tsLen: %s, subLen: %s, profLen: %d\n", argv[1], argv[2], argv[3], mlen);
#endif

  if(f == NULL){
    perror("fopen");
    exit(1);
  }

  // declaration
  double *ts, *mp;
  int* mpi;
  double *mu, *df, *dg, *norm;
  double *QT, *tmp;

  double time_spent = 0.0;
  double begin, end;

  // allocation
  ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );
  
  mu = (double*) malloc(mlen * sizeof(double) );
  norm = (double*) malloc(mlen * sizeof(double) );
  df = (double*) malloc(mlen * sizeof(double) );
  dg = (double*) malloc(mlen * sizeof(double) );
  
  QT = (double*) malloc(mlen * sizeof(double) );
  tmp= (double*) malloc(mlen * sizeof(double) );

  for(int i = 0; i < len; i++){
    fscanf(f, "%lf\n", ts+i);
  }
  fclose(f);

  // initialize output
  #pragma omp parallel for schedule(static) num_threads(128)
  for(int i=0; i<mlen; i++){
    mp[i]=-1;
    mpi[i]=-1;
  }

  // prepare statistics, initialization 
  precom_mu(ts,mu,mlen,sublen);
  precom_norm(ts,norm,mu,mlen,sublen);
  precom_df(ts,df,mlen,sublen);
  precom_dg(ts,mu,dg,mlen,sublen);

  #pragma omp parallel for schedule(static) num_threads(128)
  for(int i=0;i<mlen;i++){
	  QT[i]=0;
  }

  // initialization of the first row of matrix
  int start = 0;
  #pragma omp parallel for schedule(static) num_threads(128)
  for(int i =0;i<mlen;i++){
    for (int j=0;j<sublen;j++){
      if (i-sublen >=0)
      QT[i-sublen]+= (ts[start + j] - mu[start] )*(ts[i+j]- mu[i]);
    }
  }
  
  // main computation loops
  // TODO loop tiling and parallelization

  auto retval = PAPI_hl_region_begin("computation");
  if(retval != PAPI_OK) {
    handle_error(1);
  }
  
  begin = get_curr_time();

  for (int i=0;i<mlen-sublen;i++){
    MyMax maxStruct={-1.0, -1};
    // TODO loop tiling and simd parallelization
    #pragma omp simd reduction(maximo:maxStruct)
	  for (int j=i+sublen;j<mlen ;j++){
      // streaming dot product
		  if (i!=0)
		    QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];          //4 FLOPS

      double cr = QT[j-i-sublen] * norm[i] * norm[j]; //2 FLOPS
           

     //row-wise
      if (cr > maxStruct.max_cr ) {
		    maxStruct.max_cr = cr; //assign cr to local maximums 
        maxStruct.idx = j;
		    
		  } 

      //column-wise-> NEED TO IMPROVE
	  	if (cr > mp[j]){
		    mp[j]=cr;
		    mpi[j]=i;
		  }
        
	  }
    // updating the nearest neighbors information
    if (maxStruct.max_cr > mp[i])
      {
        mp[i] = maxStruct.max_cr; //ONCE YOU HAVE COLLECTED ALL LOCAL MAXIMUMS, Perform reduction AND ASSIGN IT 
        mpi[i] = maxStruct.idx;
      }

  }



  end = get_curr_time();

  retval = PAPI_hl_region_end("computation"); 
  
  if ( retval != PAPI_OK ){
   handle_error (1);
  }

  time_spent = end - begin;

#ifdef DEB
    print_buf<double>( "ts:", ts, len);
    print_buf<double>( "mp:",  mp, mlen);
    print_buf<int>( "mpi:", mpi, mlen);
    print_buf<double>( "mu:", mu, mlen);
    print_buf<double>( "norm:", norm, mlen);
    print_buf<double>( "df:", df, mlen);
    print_buf<double>( "dg:", dg, mlen);
    print_buf<double>( "QT:", QT, mlen);
#endif

  // so far the cacluate mp is based on pearson correlation
  // we perform the following loop to convert mp to ED.
  #pragma omp parallel for schedule(static) num_threads(128)
  for (int j=0; j<mlen;j++)
    mp[j] = sqrt( 2 * sublen * (1 - mp[j]) );

  // write the results
  f = fopen("output/mp.txt","w");
  for (int i=0;i<mlen;i++)
    fprintf(f, "%f\n", mp[i]);
  fclose(f);

  f = fopen("output/mpi.txt","w");
  for (int i=0;i<mlen;i++)
    fprintf(f, "%d\n", mpi[i]);
  fclose(f);

  // deallocation
  free (ts); free(mp); free(mpi); free(mu); free(norm); free(df); free(dg); free(QT); free(tmp);

  printf("time spent on main kernel = %f \n", time_spent);


  return 0;
}
