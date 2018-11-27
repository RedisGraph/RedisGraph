//------------------------------------------------------------------------------
// GB_subassign_scalar: C(Rows,Cols)<Mask> = accum (C(Rows,Cols),x)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Assigns a single scalar to a submatrix:
// C(Rows,Cols)<Mask> = accum (C(Rows,Cols),x)
// This function does the work for GxB_Matrix_subassign_TYPE and
// GxB_Vector_subassign_[type], where [type] is one of the 11 types, or the
// type-generic macro suffix, "_scalar".

// Compare with GB_assign_scalar, which uses Mask and C_replace differently

#include "GB.h"

GrB_Info GB_subassign_scalar        // C(Rows,Cols)<Mask> += x
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // mask for C(Rows,Cols), unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const void *scalar,             // scalar to assign to C(Rows,Cols)
    const GB_Type_code scalar_code, // type code of scalar to assign
    const GrB_Index *Rows,          // row indices
    const GrB_Index nRows,          // number of row indices
    const GrB_Index *Cols,          // column indices
    const GrB_Index nCols,          // number of column indices
    const GrB_Descriptor desc,      // descriptor for C(Rows,Cols) and Mask
    GB_Context Context
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (GB_ALIAS_OK (C, Mask)) ;

    GB_RETURN_IF_NULL (scalar) ;
    ASSERT (scalar_code <= GB_UDT_code) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // C(Rows,Cols)<Mask> = accum (C(Rows,Cols), scalar)
    //--------------------------------------------------------------------------

    return (GB_subassign (
        C,          C_replace,      // C matrix and its descriptor
        Mask,       Mask_comp,      // Mask matrix and its descriptor
        false,                      // do not transpose the mask
        accum,                      // for accum (C(Rows,Cols),scalar)
        NULL,       false,          // no explicit matrix A
        Rows, nRows,                // row indices
        Cols, nCols,                // column indices
        true,                       // do scalar expansion
        scalar,                     // scalar to assign, expands to become A
        scalar_code,                // type code of scalar to expand
        Context)) ;
}

