//------------------------------------------------------------------------------
// GB_to_nonhyper_test: test if a matrix should convert to non-hyperspasre
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Returns true if a hypersparse matrix should be converted to non-hypersparse.
// Returns false if the matrix is already non-hypersparse.

#include "GB.h"

bool GB_to_nonhyper_test    // test for conversion to hypersparse
(
    GrB_Matrix A,           // matrix to test
    int64_t k,              // # of non-empty vectors of A, an estimate is OK,
                            // but normally A->nvec_nonempty
    int64_t vdim            // normally A->vdim
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    //--------------------------------------------------------------------------
    // test for conversion
    //--------------------------------------------------------------------------

    if (!A->is_hyper)
    { 

        //----------------------------------------------------------------------
        // A is already non-hypersparse: no need to convert it
        //----------------------------------------------------------------------

        return (false) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // A is hypersparse; test for conversion to non-hypersparse
        //----------------------------------------------------------------------

        // get the vector dimension of this matrix
        float n = (float) vdim ;

        // get the hyper ratio for this matrix
        float r = A->hyper_ratio ;

        // ensure k is in the range 0 to n, inclusive
        k = GB_IMAX (k, 0) ;
        k = GB_IMIN (k, n) ;

        return (n <= 1 || (((float) k) > n * r * 2)) ;
    }
}

