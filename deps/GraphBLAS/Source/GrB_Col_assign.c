//------------------------------------------------------------------------------
// GrB_Col_assign:    C<M>(Rows,col) = accum (C(Rows,col),u)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compare with GxB_Col_subassign, which uses the M and C_replace differently

#include "GB_assign.h"

GrB_Info GrB_Col_assign             // C<M>(Rows,col) = accum (C(Rows,col),u)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector M,             // mask for C(:,col), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(Rows,col),t)
    const GrB_Vector u,             // input vector
    const GrB_Index *Rows,          // row indices
    GrB_Index nRows,                // number of row indices
    GrB_Index col,                  // column index
    const GrB_Descriptor desc       // descriptor for C(:,col) and M
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Col_assign (C, M, accum, u, Rows, nRows, col, desc)") ;
    GB_BURBLE_START ("GrB_assign") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    ASSERT (M == NULL || GB_VECTOR_OK (M)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // C(Rows,col)<M> = accum (C(Rows,col), u)
    //--------------------------------------------------------------------------

    // construct the column index list Cols = [ col ] of length nCols = 1
    GrB_Index Cols [1] ;
    Cols [0] = col ;

    info = GB_assign (
        C,                  C_replace,      // C matrix and its descriptor
        (GrB_Matrix) M, Mask_comp, Mask_struct, // mask and its descriptor
        false,                              // do not transpose the mask
        accum,                              // for accum (C(Rows,col),u)
        (GrB_Matrix) u,     false,          // u as a matrix; never transposed
        Rows, nRows,                        // row indices
        Cols, 1,                            // a single column index
        false, NULL, GB_ignore_code,        // no scalar expansion
        true, false,                        // GrB_Col_assign
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}
