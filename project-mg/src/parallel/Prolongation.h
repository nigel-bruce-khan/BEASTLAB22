/* Simple multigrid solver on unit square, using Jacobi smoother
 * (c) 2014 Philipp Neumann, TUM I-5
 */

#ifndef _PROLONGATION_H_
#define _PROLONGATION_H_

#include "Definitions.h"
#include <iostream>

//#define NUM_THREADS 128

// standard prolongation algorithm.
class alignas(32) Prolongation
{
public:
  Prolongation(const unsigned int nxCoarseGrid, const unsigned int nyCoarseGrid)
      : _nxCoarseGrid(nxCoarseGrid), _nyCoarseGrid(nyCoarseGrid),
        _nxFineGrid(2 * nxCoarseGrid + 1),
        _nyFineGrid(2 * nyCoarseGrid + 1)
  {}

  ~Prolongation() {}

  void interpolation(const FLOAT *const coarseGrid, FLOAT *const fineGrid) const
  {
    // define pointers to all neighbouring grid points centered around the
    // inner fine grid point at (2,2)
    FLOAT *fine_SW = fineGrid + (_nxFineGrid + 3);
    FLOAT *fine_S = fine_SW + 1;
    FLOAT *fine_SE = fine_S + 1;
    FLOAT *fine_W = fine_SW + (_nxFineGrid + 2);
    FLOAT *fine_C = fine_W + 1;
    FLOAT *fine_E = fine_C + 1;
    FLOAT *fine_NW = fine_W + (_nxFineGrid + 2);
    FLOAT *fine_N = fine_NW + 1;
    FLOAT *fine_NE = fine_N + 1;

    // define pointer to coarse grid point at (1,1)
    const FLOAT *coarse = coarseGrid + (_nxCoarseGrid + 3);

    // use pos-values for direct access to fine and coarse grid point data

    unsigned int finePos_idx=0; 
    unsigned int coarsePos_idx=0;
    

    #pragma omp parallel for schedule(static) firstprivate (finePos_idx, coarsePos_idx) num_threads(128)
    for ( unsigned int y = 0; y < _nyCoarseGrid; y++)
    {
      for ( unsigned int x = 0; x < _nxCoarseGrid; x++)
      {
        finePos_idx =  (2 * y) * (_nxFineGrid+2) + (2 * x);
        coarsePos_idx = x + y * (_nxCoarseGrid + 2);

        *(fine_SW+finePos_idx) += 0.25 * (*(coarse+coarsePos_idx));
        *(fine_S+finePos_idx) +=  0.5 * (*(coarse+coarsePos_idx));
        *(fine_SE+finePos_idx) += 0.25 * (*(coarse+coarsePos_idx));
        *(fine_W+finePos_idx) +=  0.5 * (*(coarse+coarsePos_idx));
        *(fine_C+finePos_idx) +=  *(coarse+coarsePos_idx);
        *(fine_E+finePos_idx) +=  0.5 * (*(coarse+coarsePos_idx));
        *(fine_NW+finePos_idx) += 0.25 * (*(coarse+coarsePos_idx));
        *(fine_N+finePos_idx) +=  0.5 * (*(coarse+coarsePos_idx));
        *(fine_NE+finePos_idx) += 0.25 * (*(coarse+coarsePos_idx));

      }
     
    }
  }

private:
  // number of inner coarse grid points
  const unsigned int _nxCoarseGrid;
  const unsigned int _nyCoarseGrid;
  // number of inner fine grid points
  const unsigned int _nxFineGrid;
  const unsigned int _nyFineGrid;
};

#endif // _PROLONGATION_H_
