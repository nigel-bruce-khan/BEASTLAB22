#include <cstdio>
#include <cstdlib>
#include <iostream>

int main(){
    int len = 128;
    int sublen = 10;
    int mlen = len - sublen + 1;
    FILE* f_mp = fopen("output/mp.txt", "r");
    FILE* f_mp_ref = fopen("mp_ref.txt", "r");
    FILE* f_mpi = fopen("output/mpi.txt", "r");
    FILE* f_mpi_ref = fopen("mpi_ref.txt", "r");
    double* mp = (double*) malloc(len * sizeof(double) );
    double* mp_ref = (double*) malloc(len * sizeof(double) );
    int* mpi = (int*) malloc(mlen * sizeof(int) );
    int* mpi_ref = (int*) malloc(mlen * sizeof(int) );

    for(int i = 0; i < len; i++){
        fscanf(f_mp, "%lf\n", mp+i);
        fscanf(f_mp_ref, "%lf\n", mp_ref+i);
        fscanf(f_mpi, "%d\n", mpi+i);
        fscanf(f_mpi_ref, "%d\n", mpi_ref+i);
    }
    fclose(f_mp);
    fclose(f_mp_ref);
    fclose(f_mpi);
    fclose(f_mpi_ref);

    double error_mp = 0;
    int error_mpi = 0;

    for(int i = 0; i < mlen; i++){
        error_mp += abs(*(mp+i) - *(mp_ref+i));
        error_mpi += abs(*(mpi+i) - *(mpi_ref+i));
    }

    std::cout << "error mp : " << error_mp << std::endl;
    std::cout << "error mpi : " << error_mpi << std::endl;
}
