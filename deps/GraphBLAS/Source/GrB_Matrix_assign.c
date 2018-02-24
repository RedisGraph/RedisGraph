//------------------------------------------------------------------------------
// GrB_Matrix_assign: matrix assignment: C<Mask>(I,J) = accum (C(I,J),A)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB.h"

GrB_Info GrB_Matrix_assign          // C<Mask>(I,J) = accum (C(I,J),A)
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix Mask,          // optional mask for C, unused if NULL
    const GrB_BinaryOp accum,       // optional accum for Z=accum(C(I,J),T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *I,             // row indices
    const GrB_Index ni,             // number of row indices
    const GrB_Index *J,             // column indices
    const GrB_Index nj,             // number of column indices
    const GrB_Descriptor desc       // descriptor for C, Mask, and A
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    WHERE ("GrB_Matrix_assign (C, Mask, accum, A, I, ni, J, nj, desc)") ;

    RETURN_IF_NULL_OR_UNINITIALIZED (C) ;
    RETURN_IF_UNINITIALIZED (Mask) ;
    RETURN_IF_NULL_OR_UNINITIALIZED (A) ;

    // get the descriptor
    GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, A_transpose, ignore) ;

    //--------------------------------------------------------------------------
    // C<Mask>(I,J) = accum (C(I,J), A) and variations
    //--------------------------------------------------------------------------

    return (GB_assign (
        C,          C_replace,      // C matrix and its descriptor
        Mask,       Mask_comp,      // Mask matrix and its descriptor
        accum,                      // for accum (C(I,J),A)
        A,          A_transpose,    // A and its descriptor
        I, ni,                      // row indices
        J, nj,                      // column indices
        false, NULL, 0,             // no scalar expansion
        false, false)) ;            // not GrB_Col_assign nor GrB_row_assign
}

