//------------------------------------------------------------------------------
// GB_to_hyper_conform: conform a matrix to its desired hypersparse format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input matrix can have shallow A->p and/or A->h components.  If the
// hypersparsity is changed, these components are no longer shallow.  If the
// method fails and the matrix is shallow, all content is removed or freed.

#include "GB.h"

GrB_Info GB_to_hyper_conform    // conform a matrix to its desired format
(
    GrB_Matrix A,               // matrix to conform
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // GB_subref_numeric can return a matrix with jumbled columns, since it
    // soon be transposed (and sorted) in GB_accum_mask.  However, it passes
    // the jumbled matrix to GB_to_hyper_conform.  This function does not
    // access the row indices at all, so it works fine if the columns have
    // jumbled row indices.

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A to conform", GB0)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // convert to hypersparse or non-hypersparse
    //--------------------------------------------------------------------------

    GrB_Info info = GrB_SUCCESS ;

    if (GB_to_hyper_test (A, A->nvec_nonempty, A->vdim))
    { 
        info = GB_to_hyper (A, Context) ;
    }
    else if (GB_to_nonhyper_test (A, A->nvec_nonempty, A->vdim))
    { 
        info = GB_to_nonhyper (A, Context) ;
    }
    else
    { 
        // leave the matrix as-is
        ;
    }

    if (info != GrB_SUCCESS)
    { 
        // out of memory; all content has been freed
        ASSERT (A->magic == GB_MAGIC2) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_OK_OR_JUMBLED (GB_check (A, "A conformed", GB0)) ;
    return (GrB_SUCCESS) ;
}

