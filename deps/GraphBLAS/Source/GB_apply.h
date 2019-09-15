//------------------------------------------------------------------------------
// GB_apply.h: definitions for GB_apply
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_APPLY_H
#define GB_APPLY_H
#include "GB.h"

GrB_Info GB_apply                   // C<M> = accum (C, op(A)) or op(A')
(
    GrB_Matrix C,                   // input/output matrix for results
    const bool C_replace,           // C descriptor
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const bool Mask_comp,           // M descriptor
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GrB_UnaryOp op,           // operator to apply to the entries
    const GrB_Matrix A,             // first input:  matrix A
    bool A_transpose,               // A matrix descriptor
    GB_Context Context
) ;

void GB_apply_op            // apply a unary operator, Cx = op ((xtype) Ax)
(
    GB_void *restrict Cx,           // output array, of type op->ztype
    const GrB_UnaryOp op,           // operator to apply
    const GB_void *restrict Ax,     // input array, of type Atype
    const GrB_Type Atype,           // type of Ax
    const int64_t anz,              // size of Ax and Cx
    GB_Context Context
) ;

GrB_Info GB_shallow_op      // create shallow matrix and apply operator
(
    GrB_Matrix *Chandle,    // output matrix C, of type op->ztype
    const bool C_is_csc,    // desired CSR/CSC format of C
    const GrB_UnaryOp op,   // operator to apply
    const GrB_Matrix A,     // input matrix to typecast
    GB_Context Context
) ;

#endif
