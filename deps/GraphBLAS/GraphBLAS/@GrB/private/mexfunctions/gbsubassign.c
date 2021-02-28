//------------------------------------------------------------------------------
// gbsubassign: assign entries into a GraphBLAS matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// gbsubassign is an interface to GxB_Matrix_subassign and
// GxB_Matrix_assign_[TYPE], computing the GraphBLAS expression:

//      C(I,J)<#M,replace> = accum (C(I,J), A) or accum(C(I,J), A')

// where A can be a matrix or a scalar.

// Usage:

//      Cout = gbsubassign (Cin, M, accum, A, I, J, desc)

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
    gb_assign (nargout, pargout, nargin, pargin, true,
        "usage: Cout = GrB.subassign (Cin, M, accum, A, I, J, desc)") ;
}

