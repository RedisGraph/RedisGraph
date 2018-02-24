//------------------------------------------------------------------------------
// GxB_Col_subassign: C(I,j)<mask> = accum (C(I,j),u)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Col_subassign          // C(I,j)<mask> = accum (C(I,j),u)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(I,j), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(I,j),t)
    const GrB_Vector u,             // input vector
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index j,              // column index
    const GrB_Descriptor desc       // descriptor for C(I,j) and mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Col_subassign (C, mask, accum, u, I, ni, j, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore1, ignore2) ;

    //--------------------------------------------------------------------------
    // C(I,j)<mask> = accum (C(I,j), u) and variations
    //--------------------------------------------------------------------------

    // construct the column index list J = [ j ] of length nj = 1
    GrB_Index J [1] ;
    J [0] = j ;

    return (GB_subassign (
        C,                  C_replace,      // C matrix and its descriptor
        (GrB_Matrix) mask,  Mask_comp,      // mask and its descriptor
        accum,                              // for accum (C(I,j),u)
        (GrB_Matrix) u,     false,          // u as a matrix; never transposed
        I, ni,                              // row indices
        J, 1,                               // column indices
        false, NULL, 0)) ;                  // no scalar expansion
}

