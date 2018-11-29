//------------------------------------------------------------------------------
// GrB_Vector_extract: w<mask> = accum (w, u(I))
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Vector_extract         // w<mask> = accum (w, u(I))
(
    GrB_Vector w,                   // input/output vector for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w,t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *I,             // row indices
    GrB_Index ni,                   // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Vector_extract (w, mask, accum, u, I, ni, desc)") ;
    GB_RETURN_IF_NULL_OR_FAULTY (w) ;
    GB_RETURN_IF_FAULTY (mask) ;
    GB_RETURN_IF_NULL_OR_FAULTY (u) ;
    ASSERT (GB_VECTOR_OK (w)) ;
    ASSERT (mask == NULL || GB_VECTOR_OK (mask)) ;
    ASSERT (GB_VECTOR_OK (u)) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, xx1, xx2, xx3) ;

    //--------------------------------------------------------------------------
    // extract entries
    //--------------------------------------------------------------------------

    // If a column list J is constructed containing the single column index 0,
    // then T = A(I,0) followed by C<Mask>=accum(C,T) does the right thing
    // where all matrices (C, Mask, and T) are columns of size ni-by-1.  Thus,
    // GB_extract does the right thing for this case.  Note that the input u is
    // not transposed.  All GrB_Matrix objects will be in CSC format, and no
    // matrices are transposed via the C_is_vector option in GB_extract.

    // construct the column index list J = [ 0 ] of length nj = 1
    GrB_Index J [1] ;
    J [0] = 0 ;

    //--------------------------------------------------------------------------
    // do the work in GB_extract
    //--------------------------------------------------------------------------

    return (GB_extract (
        (GrB_Matrix) w,     C_replace,  // w as a matrix, and its descriptor
        (GrB_Matrix) mask,  Mask_comp,  // mask a matrix, and its descriptor
        accum,                          // optional accum for z=accum(w,t)
        (GrB_Matrix) u,     false,      // u as matrix; never transposed
        I, ni,                          // row indices I and length ni
        J, 1,                           // one column index, nj = 1
        Context)) ;
}

