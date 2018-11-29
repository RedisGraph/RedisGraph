//------------------------------------------------------------------------------
// GrB_Row_assign:    C<mask'>(row,Cols) = accum (C(row,Cols),u')
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compare with GxB_Row_subassign, which uses Mask and C_replace differently

#include "GB.h"

GrB_Info GrB_Row_assign             // C<mask'>(row,Cols) += u'
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // mask for C(row,:), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(row,Cols),t)
    const GrB_Vector u,             // input vector
    GrB_Index row,                  // row index
    const GrB_Index *Cols,          // column indices
    GrB_Index nCols,                // number of column indices
    const GrB_Descriptor desc       // descriptor for C(row,:) and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Row_assign (C, mask, accum, u, row, Cols, nCols, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (mask) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    ASSERT (mask == NULL || GB_VECTOR_OK (mask)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // C<mask'>(row,Cols) = accum (C(row,Cols), u')
    //--------------------------------------------------------------------------

    // construct the row index list Rows = [ row ] of length nRows = 1
    GrB_Index Rows [1] ;
    Rows [0] = row ;

    return (GB_assign (
        C,                  C_replace,      // C matrix and its descriptor
        (GrB_Matrix) mask,  Mask_comp,      // mask and its descriptor
        true,                               // transpose the mask
        accum,                              // for accum (C(Rows,col),u)
        (GrB_Matrix) u,     true,           // u as a matrix; always transposed
        Rows, 1,                            // a single row index
        Cols, nCols,                        // column indices
        false, NULL, 0,                     // no scalar expansion
        false, true,                        // GrB_Row_assign
        Context)) ;
}

