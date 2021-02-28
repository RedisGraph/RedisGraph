//------------------------------------------------------------------------------
// ktruss_mex.c:  construct k-truss of a graph
//------------------------------------------------------------------------------

// usage:  C = ktruss_mex (A,k)

// This function computes the same thing as ktruss.m.  See that function for a
// description.

#include "ktruss_def.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    
    // check inputs
    if (nargin != 2 || nargout > 2)
    {
        mexErrMsgTxt ("usage: [C,nsteps] = ktruss_mex (A,k)") ;
    }

    int64_t n = (int64_t) mxGetN (pargin [0]) ;
    int64_t *Ap = (int64_t *) mxGetJc (pargin [0]) ;
    int64_t *Ai = (int64_t *) mxGetIr (pargin [0]) ;
    int64_t k = (int64_t) mxGetScalar (pargin [1]) ;
    int64_t nnz = Ap [n] ;
    if (k < 3) mexErrMsgTxt ("k must be >= 3") ;

    // create the output matrix
    pargout [0] = mxCreateSparse (n, n, nnz, mxREAL) ;
    int64_t *Cp = (int64_t *) mxGetJc (pargout [0]) ;
    int64_t *Ci = (int64_t *) mxGetIr (pargout [0]) ;
    int64_t *Ce = (int64_t *) mxMalloc ((nnz+1) * sizeof (int64_t)) ;

    memcpy (Cp, Ap, (n+1) * sizeof (int64_t)) ;
    memcpy (Ci, Ai, (nnz) * sizeof (int64_t)) ;

    // construct the ktruss
    int64_t nsteps = ktruss (Cp, Ci, Ce, n, k-2, 1, n) ;

    // copy the edge weights to the output matrix
    double *Cx = mxGetPr (pargout [0]) ;
    for (int64_t p = 0 ; p < Ap [n] ; p++)
    {
        Cx [p] = (double) (Ce [p]) ;
    }
    mxFree (Ce) ;

    if (nargout > 1)
    {
        pargout [1] = mxCreateDoubleScalar ((double) nsteps) ;
    }
}

