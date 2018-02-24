//------------------------------------------------------------------------------
// GxB_Row_subassign: C(i,J)<mask'> = accum (C(i,J),u')
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GxB_Row_subassign          // C(i,J)<mask'> = accum (C(i,J),u')
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for C(i,J), unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(C(i,J),t)
    const GrB_Vector u,             // input vector
    const GrB_Index i,              // row index
    const GrB_Index *J,             // column indices
    const GrB_Index nj,             // number of column indices
    const GrB_Descriptor desc       // descriptor for C(i,J) and mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GxB_Row_subassign (C, mask, accum, u, i, J, nj, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ; 

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore1, ignore2) ;

    GrB_Index nj2 = nj ;
    if (J == GrB_ALL)
    {
        // if J is GrB_ALL, this denotes that J is ":"
        nj2 = C->ncols ;
    }

    // check the mask, must by nj2-by-1
    info = GB_Mask_compatible ((GrB_Matrix) mask, NULL, nj2, 1) ;
    if (info != GrB_SUCCESS)
    {
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // C(i,J)<mask'> = accum (C(i,J), u') and variations
    //--------------------------------------------------------------------------

    // Row assignment is not as fast as column assignment in
    // SuiteSparse:GraphBLAS

    // mask and u are column vectors (nj2-by-1) and they are explicitly
    // transposed into 1-by-nj2 matrices.  This takes O(nj2) time and space,
    // but the row assignment will have to take that much time and space
    // anyway, given the data structure

    // construct the row index list I = [ i ] of length ni = 1
    GrB_Index I [1] ;
    I [0] = i ;

    // explicitly transpose the Mask, if present, also cast to bool
    GrB_Matrix Mask = NULL ;
    if (mask != NULL)
    {
        GB_NEW (&Mask, GrB_BOOL, 1, nj2, false, true) ;
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

    info = GB_subassign (
        C,      C_replace,      // C matrix and its descriptor
        Mask,   Mask_comp,      // mask and its descriptor
        accum,                  // for accum (C(i,J),u)
        (GrB_Matrix) u, true,   // u as a matrix; always transposed
        I, 1,                   // row indices
        J, nj2,                 // column indices
        false, NULL, 0) ;       // no scalar expansion

    GB_MATRIX_FREE (&Mask) ;
    return (info) ;             // pass into directly from GB_subassign
}

