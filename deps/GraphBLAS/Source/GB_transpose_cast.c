//------------------------------------------------------------------------------
// GB_transpose_cast: transpose and typecast
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The transpose is not in-place.  No operator is applied.  C = (ctype) A' is
// computed, or C = (ctype) one (A') if iso_one is true, with typecasting if
// ctype is not equal to A->type.  If iso_one is true, C is returned as an
// iso matrix, with an iso value of 1.

#include "GB_transpose.h"

GrB_Info GB_transpose_cast      // C= (ctype) A' or one (A'), not in-place
(
    GrB_Matrix C,               // output matrix C, not in place
    GrB_Type ctype,             // desired type of C
    const bool C_is_csc,        // desired CSR/CSC format of C
    const GrB_Matrix A,         // input matrix; C != A
    const bool iso_one,         // if true, C = one (A'), as iso
    GB_Context Context
)
{ 
    ASSERT (C != A && !GB_aliased (C, A)) ;

    GrB_UnaryOp op1 = (iso_one) ? GB_unop_one (ctype->code) : NULL ;

    // C = (ctype) A' if op1 is NULL, or C = (ctype) one (A')
    return (GB_transpose (C, ctype, C_is_csc, A, op1, NULL, NULL,
        false, Context)) ;
}

