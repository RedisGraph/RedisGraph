//------------------------------------------------------------------------------
// GrB_Row_assign: C<mask'>(i,J) = accum (C(i,J),u')
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Row_assign             // C<mask'>(i,J) = accum (C(i,J),u')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(i,:), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(i,J),t)
    const GrB_Vector u,             // input vector
    const GrB_Index i,              // row index
    const GrB_Index *J,             // column indices
    const GrB_Index nj,             // number of column indices
    const GrB_Descriptor desc       // descriptor for C(i,:) and mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Row_assign (C, mask, accum, u, i, J, nj, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ; 

    ASSERT_OK (GB_check (C, "C matrix for Row_assign", 0)) ;
    ASSERT_OK_OR_NULL (GB_check (mask, "mask vector for Row_assign", 0)) ;
    ASSERT_OK (GB_check (u, "u vector for Row_assign", 0)) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore1, ignore2) ;

    GrB_Index ncols = C->ncols ;

    // check the mask, must by ncols-by-1
    info = GB_Mask_compatible ((GrB_Matrix) mask, NULL, ncols, 1) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C<mask'>(i,J) = accum (C(i,J), u') and variations
    //--------------------------------------------------------------------------

    // Row assignment is not as fast as column assignment in
    // SuiteSparse:GraphBLAS

    // mask and u are column vectors (ncols-by-1) and they are explicitly
    // transposed into 1-by-ncols matrices.  This takes O(ncols) time and
    // space, but the row assignment will have to take that much time and space
    // anyway, given the data structure

    // construct the row index list I = [ i ] of length ni = 1
    GrB_Index I [1] ;
    I [0] = i ;

    // explicitly transpose the Mask, if present, also cast to bool
    GrB_Matrix Mask = NULL ;
    if (mask != NULL)
    {
        GB_NEW (&Mask, GrB_BOOL, 1, ncols, false, true) ;
        if (info != GrB_SUCCESS)
        {
            return (info) ;
        }
        info = GB_Matrix_transpose (Mask, (GrB_Matrix) mask, NULL, true) ;
        if (info != GrB_SUCCESS)
        {
            GB_MATRIX_FREE (&Mask) ;
            return (info) ;
        }
    }

    info = GB_assign (
        C,      C_replace,      // C matrix and its descriptor
        Mask,   Mask_comp,      // mask and its descriptor
        accum,                  // for accum (C(i,J),u)
        (GrB_Matrix) u, true,   // u as a matrix; always transposed
        I, 1,                   // row indices
        J, nj,                  // column indices
        false, NULL, 0,         // no scalar expansion
        false, true) ;          // GrB_row_assign

    GB_MATRIX_FREE (&Mask) ;
    return (info) ;             // pass info directly from GB_assign
}

