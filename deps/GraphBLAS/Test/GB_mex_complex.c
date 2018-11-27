//------------------------------------------------------------------------------
// GB_mex_complex: convert a real matrix into a complex one
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// If A is real, C has an all-zero imaginary part.
// If A is complex, then C = A.

// This is a sparse version of the MATLAB 'complex' function, which does not
// work for sparse matrices.  It does not use GraphBLAS at all.

#include "mex.h"
#include "matrix.h"
#include <stdlib.h>
#include <string.h>

#define USAGE "C = GB_mex_complex (A)"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{

    // check inputs
    if (nargout > 1 || nargin != 1)
    {
        mexErrMsgTxt ("Usage: " USAGE) ;
    }

    // get the input matrix
    const mxArray *A = pargin [0] ;
    if (!mxIsSparse (A))
    {
        mexErrMsgTxt ("A must be sparse") ;
    }
    int64_t *Ap = (int64_t *) mxGetJc (A) ;
    int64_t *Ai = (int64_t *) mxGetIr (A) ;
    double  *Ax = (double  *) mxGetPr (A) ;
    double  *Az = mxIsComplex (A) ? ((double *) mxGetPi (A)) : NULL ;
    int64_t m = mxGetM (A) ;
    int64_t n = mxGetN (A) ;
    int64_t anz = Ap [n] ;

    // create the output matrix
    pargout [0] = mxCreateSparse (m, n, anz+1, mxCOMPLEX) ;
    mxArray *C = pargout [0] ;
    int64_t *Cp = (int64_t *) mxGetJc (C) ;
    int64_t *Ci = (int64_t *) mxGetIr (C) ;
    double  *Cx = (double  *) mxGetPr (C) ;
    double  *Cz = (double  *) mxGetPi (C) ;

    // copy A into C
    memcpy (Cp, Ap, (n+1) * sizeof (int64_t)) ;
    memcpy (Ci, Ai, anz   * sizeof (int64_t)) ;
    memcpy (Cx, Ax, anz   * sizeof (double )) ;
    if (Az == NULL)
    {
        for (int64_t k = 0 ; k < anz ; k++) Cz [k] = 0 ;
    }
    else
    {
        memcpy (Cz, Az, anz * sizeof (double )) ;
    }
}

