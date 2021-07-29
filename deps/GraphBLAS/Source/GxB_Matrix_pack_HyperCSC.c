//------------------------------------------------------------------------------
// GxB_Matrix_pack_HyperCSC: pack a matrix in hypersparse CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

GrB_Info GxB_Matrix_pack_HyperCSC      // pack a hypersparse CSC matrix
(
    GrB_Matrix A,       // matrix to create (type, nrows, ncols unchanged)
    GrB_Index **Ap,     // col "pointers", Ap_size >= (nvec+1)*sizeof(int64_t)
    GrB_Index **Ah,     // column indices, Ah_size >= nvec*sizeof(int64_t)
    GrB_Index **Ai,     // row indices, Ai_size >= nvals(A)*sizeof(int64_t)
    void **Ax,          // values, Ax_size >= nvals(A)*(type size)
                        // or Ax_size >= (type size), if iso is true
    GrB_Index Ap_size,  // size of Ap in bytes
    GrB_Index Ah_size,  // size of Ah in bytes
    GrB_Index Ai_size,  // size of Ai in bytes
    GrB_Index Ax_size,  // size of Ax in bytes
    bool iso,           // if true, A is iso
    GrB_Index nvec,     // number of columns that appear in Ah
    bool jumbled,       // if true, indices in each column may be unsorted
    const GrB_Descriptor desc
)
{ 

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Matrix_pack_HyperCSC (A, "
        "&Ap, &Ah, &Ai, &Ax, Ap_size, Ah_size, Ai_size, Ax_size, iso, "
        "nvec, jumbled, desc)") ;
    GB_BURBLE_START ("GxB_Matrix_pack_HyperCSC") ;
    GB_RETURN_IF_NULL_OR_FAULTY (A) ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // pack the matrix
    //--------------------------------------------------------------------------

    info = GB_import (true, &A, A->type, GB_NROWS (A), GB_NCOLS (A), false,
        Ap,   Ap_size,  // Ap
        Ah,   Ah_size,  // Ah
        NULL, 0,        // Ab
        Ai,   Ai_size,  // Ai
        Ax,   Ax_size,  // Ax
        0, jumbled, nvec,                   // jumbled or not
        GxB_HYPERSPARSE, true,              // hypersparse by col
        iso, Context) ;

    GB_BURBLE_END ;
    return (info) ;
}

