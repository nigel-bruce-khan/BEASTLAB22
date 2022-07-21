#include<cstdio>
#include<cstdlib>
#include<string>
#include<cmath>
#include <iostream>
#include <omp.h>

#define NUM_THREADS 128

using namespace std;

static inline int ind(const int i, const int j, const int mlen){
	return j+i*mlen;
}

template<typename T>
void print_buf(const string desc, const T *buf, const int len){
    printf(desc.c_str());
    printf("\n");
    // no omp b/c printouts here in order
   //#pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
    for (int i=0; i<len; i++)
      //printf("%f, ",buf[i]);
      std::cout << buf[i] << ", "  ;
    printf("\n");
    printf("=========\n");
    printf("\n");
}

void precom_mu (double* ts, double* mu, int mlen, int sublen){

  double sum = 0;
 #pragma omp parallel for schedule(static) num_threads(10) reduction(+ :sum)
  for (int i=0; i<sublen; i++){
    sum+=ts[i];
  }
  mu[0]=sum/sublen;
  
  for (int i=1; i<mlen; i++){
    mu[i] = mu[i-1] + ( ts[i+sublen-1]-ts[i-1] ) / sublen;
  }
  
  return;
}

void precom_norm(double* ts, double* norm, double* mu, int mlen, int sublen){

  double sum, tmp;
  //this for loop is not working een if i add reduction sum here and not on the inner pragma, dont know why
  //#pragma omp parallel for schedule(static) num_threads(128) reduction(+ :sum)
  //#pragma omp parallel for simd reduction (+ :sum)
  for (int i=0; i<mlen; i++){
    sum = 0.0;
    tmp = 0.0;
    #pragma omp parallel for schedule(static) num_threads(10) reduction(+ :sum)
    for (int j=0; j<sublen; j++){
      tmp = ts[i+j]-mu[i];
      sum+=tmp*tmp;
    }
    norm[i] = sqrt(1.0/sum);
  }

}

void precom_df(double* ts, double* df, int mlen, int sublen){

  df[0]=0;
  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
  for (int i=0; i<mlen-1; i++){
    df[i+1] = ( ts[i+sublen]-ts[i] ) * 0.5 ;
  }

  return;
}

void precom_dg(double* ts, double* mu, double* dg, int mlen, int sublen){

  dg[0] = 0;
  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
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

  // decleration
  double *ts, *mp, *mp2, *mp3, *mp4;
  int *mpi, *mpi2, *mpi3, *mpi4;
  double *mu, *df, *dg, *norm;
  double *QT, *tmp;

  // allocation
  ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );
  mp2 = (double*) malloc(mlen * sizeof(double) );
  mpi2 = (int*) malloc(mlen * sizeof(int) );
  mp3 = (double*) malloc(mlen * sizeof(double) );
  mpi3 = (int*) malloc(mlen * sizeof(int) );
  mp4 = (double*) malloc(mlen * sizeof(double) );
  mpi4 = (int*) malloc(mlen * sizeof(int) );
  
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
  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
  for(int i=0; i<mlen; i++){
    mp[i]=-1;
    mpi[i]=-1;
    mp2[i]=-1;
    mpi2[i]=-1;
    mp3[i]=-1;
    mpi3[i]=-1;
    mp4[i]=-1;
    mpi4[i]=-1;
  }

  // prepare statistics, initialization 
  precom_mu(ts,mu,mlen,sublen);
  precom_norm(ts,norm,mu,mlen,sublen);
  precom_df(ts,df,mlen,sublen);
  precom_dg(ts,mu,dg,mlen,sublen);

  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
  for(int i =0;i<mlen;i++){
	  QT[i]=0;
  }

  // initialization of the first row of matrix
  int start = 0;
  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
  for(int i =0;i<mlen;i++){
    for (int j=0;j<sublen;j++){
      if (i-sublen >=0)
      QT[i-sublen]+= (ts[start + j] - mu[start] )*(ts[i+j]- mu[i]);
    }
  }
    
  // main computation loops
  // TODO loop tiling and parallelization
 // #pragma omp parallel for schedule(static) num_threads(NUM_THREADS) reduction(max :mp[:mlen])
double cr, cr2, cr3, cr4;
//#pragma omp parallel sections num_threads (4) shared(QT)
//{

//#pragma omp section
//*****************triangle 1**********************
//i=0:54, j=11:64
  for (int i=0;i<(mlen-sublen)/2 + 1;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
	  for (int j=i+sublen;j<(mlen+sublen)/2 + 1 ;j++){
      // streaming dot product
		  if (i!=0){
		    #pragma omp critical
		    QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];
		  }
          #pragma omp critical   
		  cr = QT[j-i-sublen];
		  cr = cr * norm[i] * norm[j];

      // updating the nearest neighbors information
		  if (cr > mp[i]){
		    mp[i]=cr;
		    mpi[i]=j;
		  }

	  	  if (cr > mp[j]){
			mp[j]=cr;
			mpi[j]=i;
		  }
	  }
  }

// #pragma omp section
//*****************triangle 2**********************
//i=55:108, j=65:118
  for (int i=(mlen-sublen)/2 + 1;i<(mlen-sublen);i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=i+sublen;j<mlen ;j++){
      // streaming dot product
		 // if (i!=0)
		  	#pragma omp critical   
		    QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];
        #pragma omp critical   
		  cr2 = QT[j-i-sublen];
		  cr2 = cr2 * norm[i] * norm[j];

      // updating the nearest neighbors information
		  if (cr2 > mp2[i]){
		    mp2[i]=cr2;
		    mpi2[i]=j;
		  }

	  	  if (cr2 > mp2[j]){
			mp2[j]=cr2;
			mpi2[j]=i;
		  }
	  }
  }  
// #pragma omp section
//*****************triangle 3**********************
//i=0:53, j=65:118  
  for (int i=0;i<(mlen-sublen)/2;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=i + sublen + (mlen-sublen)/2 + 1;j<mlen ;j++){
      // streaming dot product
		  if (i!=0){
		    #pragma omp critical   
		    QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];
		  }
		 #pragma omp critical   
		  cr3 = QT[j-i-sublen];
		  cr3 = cr3 * norm[i] * norm[j];

      // updating the nearest neighbors information
		  if (cr3 > mp3[i]){
		    mp3[i]=cr3;
		    mpi3[i]=j;
		  }

	  	  if (cr3 > mp3[j]){
			mp3[j]=cr3;
			mpi3[j]=i;
		  }
	  }
  }  
// #pragma omp section
//*****************triangle 4**********************
//i=1:54, j=65:118    
  for (int i = 1;i < (mlen-sublen)/2 + 1;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=(mlen-sublen)/2 + 1 + sublen;j<(mlen-sublen)/2 + 1 + sublen + i;j++){
      // streaming dot product
		  //if (i!=0)
		    #pragma omp critical   
		    QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];
               #pragma omp critical   
	      cr4 = QT[j-i-sublen];
	      cr4 = cr4 * norm[i] * norm[j];

      // updating the nearest neighbors information
		  if (cr4 > mp4[i]){
		    mp4[i]=cr4;
		    mpi4[i]=j;
		  }

	  	  if (cr4 > mp4[j]){
			mp4[j]=cr4;
			mpi4[j]=i;
		  }
	  }
  }    
//}  
  //last loop for compare
  for (int i=0;i<mlen-sublen;i++){
    // TODO loop tiling and simd parallelization
     // #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
	  for (int j=i+sublen;j<mlen ;j++){
	  //row comparions
	  	 if (mp2[i] > mp[i]){
		      mp[i]=mp2[i];
		      mpi[i]=mpi2[i];
		 }
		 if (mp3[i] > mp[i]){
		      mp[i]=mp3[i];
		      mpi[i]=mpi2[i];
		 } 
		 if (mp4[i] > mp[i]){
		      mp[i]=mp4[i];
		      mpi[i]=mpi4[i];
		 }
	  //column comparisons	 
		 if (mp2[j] > mp[j]){
		      mp[j]=mp2[j];
		      mpi[j]=mpi2[j];
		 }
		 if (mp3[j] > mp[j]){
		      mp[j]=mp3[j];
		      mpi[j]=mpi3[j];
		 } 
		 if (mp4[j] > mp[j]){
		      mp[j]=mp4[j];
		      mpi[j]=mpi4[j];
		 }     
	  }
  }
   

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
  #pragma omp parallel for schedule(static) num_threads(NUM_THREADS)
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
  free (ts); free(mp); free(mpi); free(mp2); free(mpi2); free(mp3); free(mpi3); free(mp4); free(mpi4); free(mu); free(norm); free(df); free(dg); free(QT); free(tmp);
  return 0;
}






/*
//original triangle divided by 2 in lenghts of row and column gives 4 smaller triangles
//*****************triangle 1**********************
//i=0:54, j=10:64
for (int i=0;i<(mlen-sublen)/2 + 1;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
	  for (int j=i+sublen;j<(mlen+sublen)/2 + 1 ;j++){
	  
	  
//*****************triangle 2**********************
//i=55:108, j=65:118
for (int i=(mlen-sublen)/2 + 1;i<(mlen-sublen);i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=i+sublen;j<mlen ;j++){
      
//*****************triangle 3**********************
//i=0:54, j=65:118  
for (int i=0;i<(mlen-sublen)/2 + 1;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=i + sublen + (mlen-sublen)/2 + 1;j<mlen ;j++){
      
//*****************triangle 4**********************
//i=1:54, j=65:118    
for (int i = 1;i < (mlen-sublen)/2 + 1;i++){
    // TODO loop tiling and simd parallelization
      //#pragma omp parallel for simd schedule(static) reduction(+ :QT[:mlen])
      for (int j=(mlen-sublen)/2 + 1 + sublen;j<(mlen-sublen)/2 + 1 + sublen + i;j++){
*/

