/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _JACOBI_H_
#define _JACOBI_H_

#include <omp.h>
#include <chrono>
#include <iostream>
#include <immintrin.h>
#include "Definitions.h"

//#define NUM_THREADS 128 

// carries out a jacobi step, reading from val and writing to _writeGrid.
class alignas(32) Jacobi
{
public:
  Jacobi(const unsigned int nx, const unsigned int ny)
      : _nx(nx), _ny(ny), _X(getX(nx, ny)), _Y(getY(nx, ny)), _RHS(getRHS(nx, ny)), _NUM_THREADS((nx < 16) ? 1 : 128) {

        std::cout << "Num Threads" << _NUM_THREADS << " Nx " <<  _nx <<  " from Jacobi " << std::endl;
      }

  ~Jacobi() {}
   
  void iterate(const FLOAT *const readField, FLOAT *const writeField, const FLOAT *const rhs) const
  {
    // set pointers of 5-point stencil (only neighbour values) to very first inner grid point
    const FLOAT *readPtr_S = readField + 1;
    const FLOAT *readPtr_W = readField + (_nx + 2);
    const FLOAT *readPtr_E = readField + (_nx + 4);
    const FLOAT *readPtr_N = readField + (2 * _nx + 5);
    const FLOAT *rhsPtr = rhs + (_nx + 3);

    FLOAT *writePtr = writeField + (_nx + 3);
    
    __m256d _rhs = _mm256_set1_pd(_RHS);
    __m256d _x = _mm256_set1_pd(_X);
    __m256d _y = _mm256_set1_pd(_Y);
    const int ub = _nx - (_nx % 4);

    //HOTSPOT
    //
    // use pos to advance access through the whole grid without any expensive index computations
   
    unsigned int pos;
    #pragma omp parallel for schedule(static) num_threads(_NUM_THREADS)
    for (unsigned int y = 1; y < _ny + 1; y++)
    {
        FLOAT buffer[4] = {0, 0, 0, 0};
        for (unsigned int x = 1; x < ub + 1; x+=4)
        {
          // do Jacobi update and write to writePtr
          pos = (y-1)*(_nx+2) + x-1;
          __m256d read_s = _mm256_loadu_pd(readField + 1 + pos);
          __m256d read_w = _mm256_loadu_pd(readField + (_nx + 2) + pos);
          __m256d read_e = _mm256_loadu_pd(readField + (_nx + 4) + pos);
          __m256d read_n = _mm256_loadu_pd(readField + (2 * _nx + 5) + pos);
          __m256d rhsptr = _mm256_loadu_pd(rhs + (_nx + 3) + pos);

          __m256d a = _mm256_mul_pd(_rhs, rhsptr);
          __m256d b = _mm256_add_pd(read_w, read_e);
          __m256d c = _mm256_add_pd(read_s, read_n);
          __m256d d = _mm256_mul_pd(_x, b);
          __m256d e = _mm256_mul_pd(_y, c);
          __m256d f = _mm256_add_pd(a, d);
          __m256d g = _mm256_add_pd(f, e);
          _mm256_store_pd(buffer, g);
          for (int q = 0; q < 4; q++) {
	       	    writePtr[pos+q] = buffer[q];
	        }
        }
        for (unsigned int x = ub+1; x < _nx+1; x++)
        {
          pos = (y-1)*(_nx+2) + x-1;
          writePtr[pos] = _RHS * rhsPtr[pos];
          writePtr[pos] += _X * (readPtr_W[pos] + readPtr_E[pos]);
          writePtr[pos] += _Y * (readPtr_S[pos] + readPtr_N[pos]);
        }
    }
  }
  //

private:
  // returns the prefactor for the Jacobi stencil in x-direction
  FLOAT getX(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hy * hy / (2.0 * (hx * hx + hy * hy));
  }
  // returns the prefactor for the Jacobi stencil in y-direction
  FLOAT getY(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return hx * hx / (2.0 * (hx * hx + hy * hy));
  }
  // returns the prefactor for the right hand side in Jacobi computation
  FLOAT getRHS(const unsigned int nx, const unsigned int ny) const
  {
    const FLOAT hx = 1.0 / (nx + 1);
    const FLOAT hy = 1.0 / (ny + 1);
    return -1.0 / (2.0 / hx / hx + 2.0 / hy / hy);
  }

  // number of inner grid points
  const unsigned int _nx;
  const unsigned int _ny;
  // prefactors in Jacobi computations
  const FLOAT _X;
  const FLOAT _Y;
  const FLOAT _RHS;
  int _NUM_THREADS;
};

#endif // _JACOBI_H_
