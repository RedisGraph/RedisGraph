//------------------------------------------------------------------------------
// GxB_Matrix_select: select entries from a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C<Mask> = accum(C,select(A,k)) or accum(C,select(A',))

#include "GB.h"

GrB_Info GxB_Matrix_select  // C<Mask> = accum (C, select(A,k)) or select(A',k)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional Mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C,T)
    const GxB_SelectOp op,          // operator to select the entries
    const GrB_Matrix A,             // first input:  matrix A
    const void *k,                  // optional input for select operator
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Matrix_select (C, Mask, accum, op, A, k, desc)") ;
    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (Mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    //--------------------------------------------------------------------------
    // select the entries and optionally transpose; assemble pending tuples
    //--------------------------------------------------------------------------

    return (GB_select (
        C,      C_replace,          // C and its descriptor
        Mask,   Mask_comp,          // Mask and its descriptor
        accum,                      // optional accum for Z=accum(C,T)
        op,                         // operator to select the entries
        A,                          // first input: A
        k,                          // optional input for select operator
        A_transpose)) ;             // descriptor for A
}

