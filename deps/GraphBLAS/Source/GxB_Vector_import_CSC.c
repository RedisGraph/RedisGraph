//------------------------------------------------------------------------------
// GxB_Vector_import_CSC: import a vector in CSC format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

GrB_Info GxB_Vector_import_CSC  // import a vector in CSC format
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length

    GrB_Index **vi,     // indices, vi_size >= nvals(v)
    void **vx,          // values, vx_size 1, or >= nvals(v)
    GrB_Index vi_size,  // size of Ai
    GrB_Index vx_size,  // size of Ax

    GrB_Index nvals,    // # of entries in vector
    bool jumbled,       // if true, indices may be unsorted
    const GrB_Descriptor desc
)
{

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Vector_import_CSC (&v, type, n, "
        " &vi, &vx, vi_size, vx_size, nvals, jumbled, desc)") ;
    GB_BURBLE_START ("GxB_Vector_import_CSC") ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // import the vector
    //--------------------------------------------------------------------------

    GrB_Index *vp = GB_MALLOC (2, int64_t) ;
    if (vp == NULL)
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }
    vp [0] = 0 ;
    vp [1] = nvals ;

    info = GB_import ((GrB_Matrix *) v, type, n, 1,
        &vp,  2,        // Ap
        NULL, 0,        // Ah
        NULL, 0,        // Ab
        vi,   vi_size,  // Ai
        vx,   vx_size,  // Ax
        0, jumbled, 0,                      // jumbled or not
        GxB_SPARSE, true, Context) ;        // sparse by col

    GB_FREE (vp) ;
    GB_BURBLE_END ;
    return (info) ;
}

