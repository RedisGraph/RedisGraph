//------------------------------------------------------------------------------
// GB_ewise.h: definitions for GB_ewise
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_EWISE_H
#define GB_EWISE_H
#include "GB.h"

GrB_Info GB_ewise                   // C<M> = accum (C, A+B) or A.*B
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // if true, clear C before writing to it
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // if true, complement the mask M
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_BinaryOp op,          // defines '+' for C=A+B, or .* for A.*B
    const GrB_Matrix A,             // input matrix
    bool A_transpose,               // if true, use A' instead of A
    const GrB_Matrix B,             // input matrix
    bool B_transpose,               // if true, use B' instead of B
    bool eWiseAdd,                  // if true, do set union (like A+B),
                                    // otherwise do intersection (like A.*B)
    GB_Context Context
) ;

#endif
