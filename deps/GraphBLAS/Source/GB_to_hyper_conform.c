//------------------------------------------------------------------------------
// GB_to_hyper_conform: conform a matrix to its desired hypersparse format
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The input matrix can have shallow A->p and/or A->h components.  If the
// hypersparsity is changed, these components are no longer shallow.  If the
// method fails and the matrix is shallow, all content is removed or freed.
// The input matrix may be jumbled; this is not an error condition.  Zombies
// are OK, but A never has pending tuples.  However, this function is agnostic
// about pending tuples so they could be OK.

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

    ASSERT_MATRIX_OK_OR_JUMBLED (A, "A to conform", GB0) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // convert to hypersparse or non-hypersparse
    //--------------------------------------------------------------------------

    GrB_Info info = GrB_SUCCESS ;

    if (A->nvec_nonempty < 0)
    { 
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

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

    ASSERT_MATRIX_OK_OR_JUMBLED (A, "A conformed", GB0) ;
    return (GrB_SUCCESS) ;
}

