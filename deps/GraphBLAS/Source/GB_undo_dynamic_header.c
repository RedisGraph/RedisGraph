//------------------------------------------------------------------------------
// GB_undo_dynamic_header: undo the dynamic header from GB_do_dynamic_header
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// If the matrix A has a static header, (*A_dynamic) is copied into it and
// then the header of A_dynamic is freed.  If A already has a dynamic header,
// then A and (*A_dynamic) are identical, and nothing is done.

#include "GB_dynamic.h"

void GB_undo_dynamic_header
(
    // input
    GrB_Matrix *A_dynamic,      // input matrix with dynamic header,
                                // NULL on output
    // output
    GrB_Matrix A,               // output matrix with static or dynamic header
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (A_dynamic == NULL || (*A_dynamic) == NULL || A == NULL ||
        !(A->static_header))
    { 
        // nothing to do
        return ;
    }

    ASSERT (!((*A_dynamic)->static_header)) ;

    //--------------------------------------------------------------------------
    // copy the dynamic header of A_dynamic back to the static header, A
    //--------------------------------------------------------------------------

    size_t header_size = (*A_dynamic)->header_size ;
    memcpy (A, *A_dynamic, sizeof (struct GB_Matrix_opaque)) ;
    A->static_header = true ;
    A->header_size = 0 ;
    GB_FREE (A_dynamic, header_size) ;

    //--------------------------------------------------------------------------
    // A now has a static header
    //--------------------------------------------------------------------------

    ASSERT ((*A_dynamic) == NULL) ;
    ASSERT (A->static_header) ;
}

