//------------------------------------------------------------------------------
// GrB_Col_extract: w<mask> = accum (w, A(I,j)) or A(j,I)'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract a single row or column from a matrix.  Note that in the
// GraphBLAS spec, a row and column are indistinguishable.  In this
// implementation, both are the same as an n-by-1 GrB_Matrix, except with
// restrictions on the matrix operations that can be performed on them.

#include "GB.h"

GrB_Info GrB_Col_extract         // w<mask> = accum (w, A(I,j)) or (A(j,I))'
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index j,              // column index
    const GrB_Descriptor desc       // descriptor for w, mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Col_extract (w, mask, accum, A, I, ni, j, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    GrB_Index ancols = (A_transpose ? A->nrows : A->ncols) ;
    if (j >= ancols)
    {
        return (ERROR (GrB_INVALID_INDEX, (LOG,
            "Column index j "GBu" out of range; must be < "GBu, j, ancols))) ;
    }

    //--------------------------------------------------------------------------
    // extract the jth column (or jth row if A is transposed) using GB_extract
    //--------------------------------------------------------------------------

    // Let A be a matrix of dimension nrows-by-ncols.

    // If A_transpose is false:

    //      I is a list of row indices, of length ni >= 0, in range 0:nrows-1.
    //      J is a single column index, of length nj == 1, in range 0:ncols-1.
    //      T = A (I,j) is computed in GB_extract, and C<Mask> = accum(C,T)
    //      constructs a C of dimension ni-by-1, using a Mask
    //      matrix of the same size.  This is identical to w and mask.

    // If A_transpose is true:

    //      I is a list of col indices, of length ni >= 0, in range 0:ncols-1.
    //      J is a single row index j,  of length nj == 1, in range 0:nrows-1.
    //      T = (A (j,I))' is computed in GB_extract, which is a column of size
    //      ni-by-1 that contains entries from the jth row of A.  It is
    //      extracted by GB_extract as a 1-by-ni matrix, and then transposed to
    //      be size ni-by-1. This is exactly the right size of w and mask, so
    //      they can be typecasted to GrB_Matrix and operated on by GB_extract.
    //      Note that this operation explicitly transposes a row into a column
    //      and is the only place in SuiteSparse:GraphBLAS where a GrB_Vector
    //      is transposed.

    // Thus, GB_extract does the right thing for both cases.

    // construct the column index list J = [ j ] of length nj = 1
    GrB_Index J [1] ;
    J [0] = j ;

    //--------------------------------------------------------------------------
    // do the work in GB_extract
    //--------------------------------------------------------------------------

    return (GB_extract (
        (GrB_Matrix) w,    C_replace,   // w as a matrix, and descriptor
        (GrB_Matrix) mask, Mask_comp,   // mask a matrix, and its descriptor
        accum,                          // optional accum for z=accum(w,t)
        A,                 A_transpose, // A and its descriptor
        I, ni,                          // row indices I and length ni
        J, 1)) ;                        // one column index, nj = 1
}

