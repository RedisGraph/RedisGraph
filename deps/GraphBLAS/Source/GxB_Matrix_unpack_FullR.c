//------------------------------------------------------------------------------
// GxB_Matrix_unpack_FullR: unpack a full matrix, held by row
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_unpack_FullR  // unpack a full matrix, by row
(
    GrB_Matrix A,       // matrix to unpack (type, nrows, ncols unchanged)
    void **Ax,          // values
    GrB_Index *Ax_size, // size of Ax in bytes
    bool *iso,          // if true, A is iso
    const GrB_Descriptor desc
)
{

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Matrix_unpack_FullR (A, "
        "&Ax, &Ax_size, &iso, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_unpack_FullR") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // finish any pending work
    //--------------------------------------------------------------------------

    GB_MATRIX_WAIT (A) ;
    if (!GB_is_dense (A))
    { 
        // A must be dense or full
        return (GrB_INVALID_VALUE) ;
    }

    //--------------------------------------------------------------------------
    // ensure the matrix is full by-row
    //--------------------------------------------------------------------------

    // ensure the matrix is in by-row format
    if (A->is_csc)
    { 
        // A = A', done in-place, to put A in by-row format
        GBURBLE ("(transpose) ") ;
        GB_OK (GB_transpose_in_place (A, false, Context)) ;
        GB_MATRIX_WAIT (A) ;
    }

    GB_convert_any_to_full (A) ;

    //--------------------------------------------------------------------------
    // unpack the matrix
    //--------------------------------------------------------------------------

    ASSERT (GB_IS_FULL (A)) ;
    ASSERT (!(A->is_csc)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    int sparsity ;
    bool is_csc ;
    GrB_Type type ;
    GrB_Index vlen, vdim ;

    info = GB_export (true, &A, &type, &vlen, &vdim, false,
        NULL, NULL,     // Ap
        NULL, NULL,     // Ah
        NULL, NULL,     // Ab
        NULL, NULL,     // Ai
        Ax,   Ax_size,  // Ax
        NULL, NULL, NULL,
        &sparsity, &is_csc,                 // full by row
        iso, Context) ;

    if (info == GrB_SUCCESS)
    {
        ASSERT (sparsity == GxB_FULL) ;
        ASSERT (!is_csc) ;
    }
    GB_BURBLE_END ;
    return (info) ;
}

