//------------------------------------------------------------------------------
// GB_mex_omp_max_threads: omp_get_max_threads ( )
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns the # of threads reported by omp_get_max_threads, if GraphBLAS was
// compiled with OpenMP.  Otherwise, returns 1.

#include "GB_mex.h"

#define USAGE "nthreads_max = GB_mex_omp_max_threads ;"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    int omp_nthreads_max = GB_Global_omp_get_max_threads ( ) ;
    pargout [0] = mxCreateDoubleScalar (omp_nthreads_max) ;
}

