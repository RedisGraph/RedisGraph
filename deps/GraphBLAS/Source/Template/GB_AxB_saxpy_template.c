//------------------------------------------------------------------------------
// GB_AxB_saxpy_template: C=A*B, C<M>=A*B, or C<!M>=A*B via saxpy method
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// All 4 matrices have any format: hypersparse, sparse, bitmap, or full.

{
    if (GB_IS_SPARSE (C) || GB_IS_HYPERSPARSE (C))
    { 
        // C is sparse or hypersparse
        #include "GB_AxB_saxpy3_template.c"
    }
    else
    { 
        // C is bitmap or full
        #include "GB_bitmap_AxB_saxpy_template.c"
    }
}

