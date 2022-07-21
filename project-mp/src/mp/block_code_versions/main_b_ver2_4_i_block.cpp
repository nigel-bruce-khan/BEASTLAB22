#include<cstdio>
#include<cstdlib>
#include<string>
#include<cmath>
#include <iostream>
#include <immintrin.h>
#include <algorithm>

#define BLOCKSIZE 70

using namespace std;

static inline int ind(const int i, const int j, const int mlen){
	return j+i*mlen;
}

double get_curr_time(){
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  return t.tv_sec + t.tv_nsec * 1e-9;
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
  for (int i=0; i<mlen-1; i++){
    df[i+1] = ( ts[i+sublen]-ts[i] ) * 0.5 ;
  }

  return;
}

void precom_dg(double* ts, double* mu, double* dg, int mlen, int sublen){

  dg[0] = 0;
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

  // allocation : try to change to alligned_alloc?
  /*ts = (double*) malloc(len * sizeof(double) );
  mp = (double*) malloc(mlen * sizeof(double) );
  mpi = (int*) malloc(mlen * sizeof(int) );
  
  mu = (double*) malloc(mlen * sizeof(double) );
  norm = (double*) malloc(mlen * sizeof(double) );
  df = (double*) malloc(mlen * sizeof(double) );
  dg = (double*) malloc(mlen * sizeof(double) );
  
  QT = (double*) malloc(mlen * sizeof(double) );
  tmp= (double*) malloc(mlen * sizeof(double) );*/

  ts = (double*) aligned_alloc(64, len * sizeof(double) );
  mp = (double*) aligned_alloc(64, mlen * sizeof(double) );
  mpi = (int*) aligned_alloc(32, mlen * sizeof(int) );
  
  mu = (double*) aligned_alloc(64, mlen * sizeof(double) );
  norm = (double*) aligned_alloc(64, mlen * sizeof(double) );
  df = (double*) aligned_alloc(64, mlen * sizeof(double) );
  dg = (double*) aligned_alloc(64, mlen * sizeof(double) );
  
  QT = (double*) aligned_alloc(64, mlen * sizeof(double) );
  tmp= (double*) aligned_alloc(64, mlen * sizeof(double) );

  for(int i = 0; i < len; i++){
    fscanf(f, "%lf\n", ts+i);
  }
  fclose(f);

  // initialize output
  for(int i=0; i<mlen; i++){
    mp[i]=-1;
    mpi[i]=-1;
  }

  // prepare statistics, initialization 
  precom_mu(ts,mu,mlen,sublen);
  precom_norm(ts,norm,mu,mlen,sublen);
  precom_df(ts,df,mlen,sublen);
  precom_dg(ts,mu,dg,mlen,sublen);

  for(int i =0;i<mlen;i++){
	  QT[i]=0;
  }

  // initialization of the first row of matrix
  int start = 0;
  for(int i =0;i<mlen;i++){
    for (int j=0;j<sublen;j++){
      if (i-sublen >=0)
      QT[i-sublen]+= (ts[start + j] - mu[start] )*(ts[i+j]- mu[i]);
    }
  }

  __m256d norm_i;
  __m256d qt;
  __m256d norm_j;
  __m256d mul_c;
  __m256d cr;
  __m256d df_i;
  __m256d dg_i;
  __m256d df_j;
  __m256d dg_j;
  __m256d mul_a;
  __m256d mul_b;
  __m256d sum_a;
  __m256d qt_new;

  double time_spent = 0.0;
  double begin, end;

  
  // main computation loops
  // TODO loop tiling and parallelization

  begin = get_curr_time();
  
  //loop over rows
  for (int row_index = 0; row_index < mlen - sublen; row_index+=BLOCKSIZE){
      //for (int column_index = row_index; column_index < mlen; column_index+= BLOCKSIZE){
		   for (int i = row_index; i < std::min(row_index + BLOCKSIZE, mlen-sublen); i++){
		  //for(int i = 0; i<mlen - sublen; i++){
			df_i = _mm256_set1_pd(df[i]);
			dg_i = _mm256_set1_pd(dg[i]);
			norm_i = _mm256_set1_pd(norm[i]);
			//int ub = std::min(column_index + BLOCKSIZE + sublen, mlen) - ((std::min(column_index + BLOCKSIZE + sublen, mlen) - (i + sublen)) % 4);
			int ub = mlen - (mlen-i-sublen) % 4;
			//loop over colums
			//for (int j = column_index + sublen; j < ub; j+=4){
			for(int j = i+sublen; j < ub ; j+=4){
			  if (j >= i + sublen) {
				  df_j = _mm256_loadu_pd(df+j);
				  dg_j = _mm256_loadu_pd(dg+j);
				  qt = _mm256_loadu_pd(QT+j-i-sublen); //load QT[j-i-sublen]
				  norm_j = _mm256_loadu_pd(norm+j);
				  cr = _mm256_setzero_pd();//_mm256_set1_pd(0); // // 4 doubles set to zero
				  mul_c = _mm256_mul_pd(norm_i, norm_j);
				  qt_new = _mm256_setzero_pd();

				  if(i!=0){
				  //QT[j-i-sublen] += df[i] * dg[j] + df[j] * dg[i];
					mul_a = _mm256_mul_pd(df_i, dg_j);
					mul_b = _mm256_mul_pd(df_j, dg_i);
					sum_a = _mm256_add_pd(mul_a, mul_b);
					//qt = _mm256_add_pd(sum_a, qt);  
					qt_new = _mm256_add_pd(sum_a, qt);
					qt = qt_new;
				  }

				  _mm256_storeu_pd(QT+j-i-sublen, qt);      

				  cr = _mm256_mul_pd(qt, mul_c);
				  double *CR = (double*)&cr;

				  //update neighboring information
				  for(int idx = 0; idx <4; ++idx){

					if(CR[idx] > mp[i]){
					  mp[i] = CR[idx];
					  mpi[i] = j + idx;
					}

				   if(CR[idx] > mp[j+idx]){
					  mp[ j+ idx] = CR[idx];
					  mpi[j + idx] = i;
					}

				  }
			  }

			}
			//Remaining iterations that couldn't be vectorized
			//for (int j = ub; j < std::min(column_index + BLOCKSIZE + sublen, mlen); j++)
			for (int j = ub; j < mlen; j++)
			{
			  if (j >= i + sublen) {
				  if (i!=0)
				  	QT[j-i-sublen] +=  df[i]*dg[j] + df[j]*dg[i];

					  double COR = QT[j-i-sublen] * norm[i] * norm[j];

				  // updating the nearest neighbors information
					  if (COR > mp[i]){
						mp[i]=COR;
						mpi[i]=j;
					  }
				  	if (COR > mp[j]){
						mp[j]=COR;
						mpi[j]=i;
					  }
			  }
			}
		  
		  }
	  //}
  }

    end = get_curr_time();

    time_spent = end - begin;
//#ifdef DEB
   // print_buf<double>( "ts:", ts, len);
    print_buf<double>( "mp:",  mp, mlen);
    print_buf<int>( "mpi:", mpi, mlen);
   // print_buf<double>( "mu:", mu, mlen);
    //print_buf<double>( "norm:", norm, mlen);
   // print_buf<double>( "df:", df, mlen);
   // print_buf<double>( "dg:", dg, mlen);
   // print_buf<double>( "QT:", QT, mlen);
//#endif

  // so far the cacluate mp is based on pearson correlation
  // we perform the following loop to convert mp to ED.
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
  return 0;
}
