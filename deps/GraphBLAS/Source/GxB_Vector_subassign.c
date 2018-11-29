//------------------------------------------------------------------------------
// GxB_Vector_subassign: w(Rows)<mask> = accum (w(Rows),u)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compare with GrB_Vector_assign, which uses mask and C_replace differently

#include "GB.h"

GrB_Info GxB_Vector_subassign       // w(Rows)<mask> = accum (w(Rows),u)
(
    GrB_Vector w,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for w(Rows), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(Rows),t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *Rows,          // row indices
    GrB_Index nRows,                // number of row indices
    const GrB_Descriptor desc       // descriptor for w(Rows) and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GxB_Vector_subassign (w, mask, accum, u, Rows, nRows, desc)") ;

    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (mask) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;

    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (mask == NULL || GB_VECTOR_OK (mask)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // w(Rows)<mask> = accum (w(Rows), u) and variations
    //--------------------------------------------------------------------------

    // construct the column index list Cols = [ 0 ] of length nCols = 1
    GrB_Index Cols [1] ;
    Cols [0] = 0 ;

    return (GB_subassign (
        (GrB_Matrix) w,     C_replace,  // w vector and its descriptor
        (GrB_Matrix) mask,  Mask_comp,  // Mask matrix and its descriptor
        false,                          // do not transpose the mask
        accum,                          // for accum (C(Rows,Cols),A)
        (GrB_Matrix) u,     false,      // u as a matrix; never transposed
        Rows, nRows,                    // row indices
        Cols, 1,                        // one column index, nCols = 1
        false, NULL, 0,                 // no scalar expansion
        Context)) ;
}

