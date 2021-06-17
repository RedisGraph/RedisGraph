//------------------------------------------------------------------------------
// GxB_Vector_import_Bitmap: import a vector in bitmap format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

GrB_Info GxB_Vector_import_Bitmap // import a bitmap vector
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length

    int8_t **vb,        // bitmap, vb_size >= n
    void **vx,          // values, vx_size 1, or >= n
    GrB_Index vb_size,  // size of vb
    GrB_Index vx_size,  // size of vx

    GrB_Index nvals,    // # of entries in bitmap
    const GrB_Descriptor desc
)
{ 

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Vector_import_Bitmap (&v, type, n, "
        " &vb, &vx, vb_size, vx_size, nvals, desc)") ;
    GB_BURBLE_START ("GxB_Vector_import_Bitmap") ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // import the vector
    //--------------------------------------------------------------------------

    info = GB_import ((GrB_Matrix *) v, type, n, 1,
        NULL, 0,        // Ap
        NULL, 0,        // Ah
        vb,   vb_size,  // Ab
        NULL, 0,        // Ai
        vx,   vx_size,  // Ax
        nvals, false, 0,                    // nvals for bitmap
        GxB_BITMAP, true, Context) ;        // bitmap by col

    GB_BURBLE_END ;
    return (info) ;
}

