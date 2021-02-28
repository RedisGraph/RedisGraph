//------------------------------------------------------------------------------
// gbassign: assign entries into a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbassign is an interface to GrB_Matrix_assign and GrB_Matrix_assign_[TYPE],
// computing the GraphBLAS expression:

//      C<#M,replace>(I,J) = accum (C(I,J), A) or accum(C(I,J), A')

// where A can be a matrix or a scalar.

// Usage:

//      Cout = gbassign (Cin, M, accum, A, I, J, desc)

// Cin, A, and desc are required.  See GrB.m for more details.

#include "gb_matlab.h"

void mexFunction
(
    int nargout,
    mxArray *pargout [ ],
    int nargin,
    const mxArray *pargin [ ]
)
{
    gb_assign (nargout, pargout, nargin, pargin, false,
        "usage: Cout = GrB.assign (Cin, M, accum, A, I, J, desc)") ;
}

