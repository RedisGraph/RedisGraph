//------------------------------------------------------------------------------
// GxB_Matrix_export_HyperCSC: export a matrix in hypersparse CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

#define GB_FREE_ALL ;

GrB_Info GxB_Matrix_export_HyperCSC  // export and free a hypersparse CSC matrix
(
    GrB_Matrix *A,      // handle of matrix to export and free
    GrB_Type *type,     // type of matrix exported
    GrB_Index *nrows,   // number of rows of the matrix
    GrB_Index *ncols,   // number of columns of the matrix

    GrB_Index **Ap,     // column "pointers", Ap_size >= nvec+1
    GrB_Index **Ah,     // column indices, Ah_size >= nvec
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)
    void **Ax,          // values, Ax_size 1, or >= nvals(A)
    GrB_Index *Ap_size, // size of Ap
    GrB_Index *Ah_size, // size of Ah
    GrB_Index *Ai_size, // size of Ai
    GrB_Index *Ax_size, // size of Ax

    GrB_Index *nvec,    // number of columns that appear in Ah
    bool *jumbled,      // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
)
{ 

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Matrix_export_HyperCSC (&A, &type, &nrows, &ncols, "
        "&Ap, &Ah, &Ai, &Ax, &Ap_size, &Ah_size, &Ai_size, &Ax_size, "
        "&nvec, &jumbled, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_export_HyperCSC") ;
    GB_RETURN_IF_NULL (A) ;
    GB_RETURN_IF_NULL_OR_FAULTY (*A) ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // ensure the matrix is in CSC format
    //--------------------------------------------------------------------------

    if (!((*A)->is_csc))
    { 
        // A = A', done in-place, to put A in CSC format
        GBURBLE ("(transpose) ") ;
        GB_OK (GB_transpose (NULL, NULL, true, *A,
            NULL, NULL, NULL, false, Context)) ;
    }

    //--------------------------------------------------------------------------
    // finish any pending work
    //--------------------------------------------------------------------------

    if (jumbled == NULL)
    { 
        // the exported matrix cannot be jumbled
        GB_MATRIX_WAIT (*A) ;
    }
    else
    { 
        // the exported matrix is allowed to be jumbled
        GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (*A) ;
    }

    //--------------------------------------------------------------------------
    // ensure the matrix is hypersparse
    //--------------------------------------------------------------------------

    GB_OK (GB_convert_any_to_hyper (*A, Context)) ;

    //--------------------------------------------------------------------------
    // export the matrix
    //--------------------------------------------------------------------------

    ASSERT (GB_IS_HYPERSPARSE (*A)) ;
    ASSERT ((*A)->is_csc) ;
    ASSERT (!GB_ZOMBIES (*A)) ;
    ASSERT (GB_IMPLIES (jumbled == NULL, !GB_JUMBLED (*A))) ;
    ASSERT (!GB_PENDING (*A)) ;

    int sparsity ;
    bool is_csc ;

    info = GB_export (A, type, nrows, ncols,
        Ap,   Ap_size,  // Ap
        Ah,   Ah_size,  // Ah
        NULL, NULL,     // Ab
        Ai,   Ai_size,  // Ai
        Ax,   Ax_size,  // Ax
        NULL, jumbled, nvec,                // jumbled or not
        &sparsity, &is_csc, Context) ;      // hypersparse by col

    if (info == GrB_SUCCESS)
    {
        ASSERT (sparsity == GxB_HYPERSPARSE) ;
        ASSERT (is_csc) ;
    }
    GB_BURBLE_END ;
    return (info) ;
}

