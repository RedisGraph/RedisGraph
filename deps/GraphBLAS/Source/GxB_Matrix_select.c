//------------------------------------------------------------------------------
// GxB_Matrix_select: select entries from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<M> = accum(C,select(A,k)) or accum(C,select(A',))

#include "GB_select.h"

GrB_Info GxB_Matrix_select  // C<M> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // first input:  matrix A
    const GxB_Scalar Thunk,         // optional input for select operator
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Matrix_select (C, M, accum, op, A, Thunk, desc)") ;
    GB_BURBLE_START ("GxB_select") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2) ;

    //--------------------------------------------------------------------------
    // select the entries and optionally transpose; assemble pending tuples
    //--------------------------------------------------------------------------

    info = GB_select (
        C,      C_replace,          // C and its descriptor
        M, Mask_comp, Mask_struct,  // mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        op,                         // operator to select the entries
        A,                          // first input: A
        Thunk,                      // optional input for select operator
        A_transpose,                // descriptor for A
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}
