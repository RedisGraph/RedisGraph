//------------------------------------------------------------------------------
// GrB_Matrix_assign:    C<M>(Rows,Cols) = accum (C(Rows,Cols),A) or A'
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#include "GB_assign.h"

GrB_Info GrB_Matrix_assign          // C<M>(Rows,Cols) += A or A'
(
    GrB_Matrix C,                   // input/output matrix for results
    const GrB_Matrix M,             // mask for C, unused if NULL
    const GrB_BinaryOp accum,       // accum for Z=accum(C(Rows,Cols),T)
    const GrB_Matrix A,             // first input:  matrix A
    const GrB_Index *Rows,          // row indices
    GrB_Index nRows,                // number of row indices
    const GrB_Index *Cols,          // column indices
    GrB_Index nCols,                // number of column indices
    const GrB_Descriptor desc       // descriptor for C, M, and A
)
{ 

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GB_WHERE ("GrB_Matrix_assign"
        " (C, M, accum, A, Rows, nRows, Cols, nCols, desc)") ;
    GB_BURBLE_START ("GrB_assign") ;

    GB_RETURN_IF_NULL_OR_FAULTY (C) ;
    GB_RETURN_IF_FAULTY (M) ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;

    // get the descriptor
    GB_GET_DESCRIPTOR (info, desc, C_replace, Mask_comp, Mask_struct,
        A_transpose, xx1, xx2) ;

    //--------------------------------------------------------------------------
    // C<M>(Rows,Cols) = accum (C(Rows,Cols), A) and variations
    //--------------------------------------------------------------------------

    info = GB_assign (
        C,          C_replace,      // C matrix and its descriptor
        M, Mask_comp, Mask_struct,  // mask matrix and its descriptor
        false,                      // do not transpose the mask
        accum,                      // for accum (C(Rows,Cols),A)
        A,          A_transpose,    // A and its descriptor (T=A or A')
        Rows, nRows,                // row indices
        Cols, nCols,                // column indices
        false, NULL, GB_ignore_code,// no scalar expansion
        false, false,               // not GrB_Col_assign nor GrB_row_assign
        Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

