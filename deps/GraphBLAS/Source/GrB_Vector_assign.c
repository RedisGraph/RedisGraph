//------------------------------------------------------------------------------
// GrB_Vector_assign: w<mask>(I) = accum (w(I),u)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Vector_assign          // w<mask>(I) = accum (w(I),u)
(
    GrB_Vector w,                   // input/output matrix for results
    const GrB_Vector mask,          // optional mask for w, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for z=accum(w(I),t)
    const GrB_Vector u,             // first input:  vector u
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Descriptor desc       // descriptor for w and mask
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Vector_assign (w, mask, accum, u, I, ni, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (w) ;
    RETURN_IF_UNINITIALIZED (mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (u) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, ignore1, ignore2) ;

    //--------------------------------------------------------------------------
    // w(I)<mask> = accum (w(I), u) and variations
    //--------------------------------------------------------------------------

    // construct the column index list J = [ 0 ] of length nj = 1
    GrB_Index J [1] ;
    J [0] = 0 ;

    return (GB_assign (
        (GrB_Matrix) w,     C_replace,  // w vector and its descriptor
        (GrB_Matrix) mask,  Mask_comp,  // Mask matrix and its descriptor
        accum,                          // for accum (C(I,J),A)
        (GrB_Matrix) u,     false,      // u as a matrix; never transposed
        I, ni,                          // row indices
        J, 1,                           // one column index, nj = 1
        false, NULL, 0,                 // no scalar expansion
        false, false)) ;            // not GrB_Col_assign nor GrB_row_assign
}

