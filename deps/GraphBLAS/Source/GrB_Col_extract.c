//------------------------------------------------------------------------------
// GrB_Col_extract: w<mask> = accum (w, A(I,j)) or A(j,I)'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Extract a single row or column from a matrix.  Note that in the
// GraphBLAS spec, row and column vectors are indistinguishable.  In this
// implementation, both are the same as an n-by-1 GrB_Matrix, except with
// restrictions on the matrix operations that can be performed on them.

#include "GB.h"

GrB_Info GrB_Col_extract        // w<mask> = accum (w, A(I,j)) or (A(j,I))'
(
    GrB_Vector w,               // input/output vector for results
    const GrB_Vector mask,      // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,   // optional accum for z=accum(w,t)
    const GrB_Matrix A,         // first input:  matrix A
    const GrB_Index *I,         // row indices
    GrB_Index ni,               // number of row indices
    GrB_Index j,                // column index
    const GrB_Descriptor desc   // descriptor for w, mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Col_extract (w, mask, accum, A, I, ni, j, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (mask) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (GB_IMPLIES (mask != NULL, GB_VECTOR_OK (mask))) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, xx1, xx2);

    GrB_Index ancols = (A_transpose ? GB_NROWS (A) : GB_NCOLS (A)) ;
    if (j >= ancols)
    { 
        return (GB_ERROR (GrB_INVALID_INDEX, (GB_LOG,
            "Column index j "GBu" out of range; must be < "GBu, j, ancols))) ;
    }

    //--------------------------------------------------------------------------
    // extract the jth column (or jth row if A is transposed) using GB_extract
    //--------------------------------------------------------------------------

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
        J, 1,                           // one column index, nj = 1
        Context)) ;
}

