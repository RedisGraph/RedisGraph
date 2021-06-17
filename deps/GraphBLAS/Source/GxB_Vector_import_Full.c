//------------------------------------------------------------------------------
// GxB_Vector_import_Full: import a vector in full format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_export.h"

GrB_Info GxB_Vector_import_Full // import a full vector
(
    GrB_Vector *v,      // handle of vector to create
    GrB_Type type,      // type of vector to create
    GrB_Index n,        // vector length

    void **vx,          // values, vx_size 1, or >= nvals(v)
    GrB_Index vx_size,  // size of vx

    const GrB_Descriptor desc
)
{ 

    //--------------------------------------------------------------------------
    // check inputs and get the descriptor
    //--------------------------------------------------------------------------

    GB_WHERE1 ("GxB_Vector_import_Full (&v, type, n, "
        "&vx, vx_size, desc)") ;
    GB_BURBLE_START ("GxB_Vector_import_Full") ;
    GB_GET_DESCRIPTOR (info, desc, xx1, xx2, xx3, xx4, xx5, xx6, xx7) ;

    //--------------------------------------------------------------------------
    // import the vector
    //--------------------------------------------------------------------------

    info = GB_import ((GrB_Matrix *) v, type, n, 1,
        NULL, 0,        // Ap
        NULL, 0,        // Ah
        NULL, 0,        // Ab
        NULL, 0,        // Ai
        vx,   vx_size,  // Ax
        0, false, 0,
        GxB_FULL, true, Context) ;          // full by col

    GB_BURBLE_END ;
    return (info) ;
}

