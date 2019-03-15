//------------------------------------------------------------------------------
// GB_cumsum: cumlative sum of an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compute the cumulative sum of an array count[0:n], of size n+1
// in pseudo-MATLAB notation:

//      k = sum (count [0:n] != 0) ;

//      count = cumsum ([0 count[0:n-1]]) ;

// That is, count [j] on input is overwritten with the value of
// sum (count [0..j-1]).

// GB_transpose_bucket uses this function to compute row and column pointers.
// On input, count [j] is the number of nonzeros in column j of a matrix, and
// count [n] should be zero (it only affects the result k, however).  On
// output, count [0..n] contains the column pointers of the matrix, and kresult
// is the number of nonzeros in count [0:n], which becomes nvec_nonempty for
// the resulting matrix.

// GB_AxB_flopcount uses this to compute the cumulative sum of the flop
// counts for C<M>=A*B, for each column of B.  In this case, n is the
// number of vectors in B.  It does not need the value k, so it is not
// computed.

// PARALLEL: a parallel cumsum

#include "GB.h"

void GB_cumsum                  // compute the cumulative sum of an array
(
    int64_t *count,             // size n+1, input/output
    const int64_t n,
    int64_t *kresult,           // return k, if needed by the caller
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (count != NULL) ;
    ASSERT (n >= 0) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS (nthreads, Context) ;

    //--------------------------------------------------------------------------
    // count = cumsum ([0 count[0:n-1]]) ;
    //--------------------------------------------------------------------------

    if (kresult == NULL)
    { 
        // do not compute k.  This loop is essential to parallelize.
        int64_t s = 0 ;
        for (int64_t i = 0 ; i <= n ; i++)
        { 
            int64_t c = count [i] ;
            count [i] = s ;
            s += c ;
        }
    }
    else
    { 
        // also compute k as the # of nonzeros in count [0:n].  Note that
        // the bucket transpose need not be parallel, so it is not essential
        // to parallelize the following code.
        int64_t k = 0 ;
        int64_t s = 0 ;
        for (int64_t i = 0 ; i <= n ; i++)
        { 
            int64_t c = count [i] ;
            if (c != 0) k++ ;
            count [i] = s ;
            s += c ;
        }
        (*kresult) = k ;
    }
}

