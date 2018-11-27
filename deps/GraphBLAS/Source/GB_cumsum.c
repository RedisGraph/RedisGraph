//------------------------------------------------------------------------------
// GB_cumsum: cumlative sum of an array
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Compute the cumulative sum of an array count[0:n], of size n+1
// in pseudo-MATLAB notation:
//
//      k = sum (count [0:n] != 0) ;
//      count = cumsum ([0 count[0:n-1]]) ;
//      p = count ;
//
// Note that count [n] does not appear in the output count, although it does
// appear in s.  GraphBLAS uses this function to compute row and column
// pointers.  On input, count [j] is the number of nonzeros in column j of a
// matrix, and count [n] is zero.  On output, p [0..n] contains the column
// pointers of the matrix.  k is the number of nonzeros in count [0:n].

#include "GB.h"

int64_t GB_cumsum               // compute the cumulative sum of an array
(
    int64_t *p,                 // size n+1, undefined on input
    int64_t *count,             // size n+1, input/output
    const int64_t n
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (count != NULL) ;
    ASSERT (n >= 0) ;

    //--------------------------------------------------------------------------
    // count = cumsum ([0 count[0:n-1]]) ;
    //--------------------------------------------------------------------------

    int64_t k = 0 ;
    int64_t s = 0 ;
    for (int64_t i = 0 ; i <= n ; i++)
    { 
        int64_t c = count [i] ;
        if (c != 0) k++ ;
        count [i] = s ;
        s += c ;
    }

    //--------------------------------------------------------------------------
    // p [0:n] = count [0:n], but skip this step is p is NULL
    //--------------------------------------------------------------------------

    if (p != NULL)
    { 
        memcpy (p, count, (n+1) * sizeof (int64_t)) ;
    }

    //--------------------------------------------------------------------------
    // return number of nonzeros in count [0:n]
    //--------------------------------------------------------------------------

    return (k) ;
}

