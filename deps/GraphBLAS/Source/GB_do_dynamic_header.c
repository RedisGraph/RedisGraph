//------------------------------------------------------------------------------
// GB_do_dynamic_header: ensure a matrix has a dynamic header, not static
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// The A matrix has either a dynamic or static header.  If it has a dynamic
// header, (*A_dynamic) is returned as A.

// If A has a static header, then then a copy of the header is allocated
// dynamically and the contents of A are copied into A.  The A matrix is not
// modified in either case.

// (*A_dynamic) is not a shallow copy of A, but an exact alias.  If any work is
// done to modify (*A_dynamic), this work is copied back to A when done, by
// GB_undo_dynamic_header.

// Once a method is done with A, it should free it, as follows.

//      GrB_Matrix A ;
//      GB_OK (GB_do_dynamic_header (&A_dynamic, A, Context)) ;
//      ... use A_dynamic instead of A ...
//      GB_undo_dynamic_header (&A_dynamic, A, Context) ;

#include "GB_dynamic.h"

GrB_Info GB_do_dynamic_header
(
    // output
    GrB_Matrix *A_dynamic,      // copy of A but with dynamic header
    // input
    GrB_Matrix A,               // input with static or dynamic header
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A_dynamic != NULL) ;
    (*A_dynamic) = NULL ;

    //--------------------------------------------------------------------------
    // quick return if no input matrix
    //--------------------------------------------------------------------------

    if (A == NULL)
    { 
        // nothing to do; this is not an error condition, since A might
        // be an optional matrix
        return (GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // convert A to a dynamic header, if necessary 
    //--------------------------------------------------------------------------

    if (A->static_header)
    {
        // allocate a new dynamic header for (*A_dynamic) and copy A into it
        size_t header_size ;
        (*A_dynamic) = GB_MALLOC (1, struct GB_Matrix_opaque, &header_size) ;
        if ((*A_dynamic) == NULL)
        { 
            // out of memory
            return (GrB_OUT_OF_MEMORY) ;
        }
        // copy the contents of the static header into the dynamic header
        memcpy ((*A_dynamic), A, sizeof (struct GB_Matrix_opaque)) ;
        (*A_dynamic)->static_header = false ;
        (*A_dynamic)->header_size = header_size ;
    }
    else
    { 
        // A already has a dynamic header, so (*A_dynamic) is just A
        (*A_dynamic) = A ;
    }

    //--------------------------------------------------------------------------
    // A_dynamic now has a dynamic header
    //--------------------------------------------------------------------------

    ASSERT (!(*A_dynamic)->static_header) ;
    return (GrB_SUCCESS) ;
}

