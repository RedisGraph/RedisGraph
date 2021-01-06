//------------------------------------------------------------------------------
// GB_transpose_method: select method for GB_transpose
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

#include "GB_transpose.h"

// GB_transpose can use choose between a merge-sort-based method that takes
// O(anz*log(anz)) time, or a bucket-sort method that takes O(anz+m+n) time.
// The bucket sort has 3 methods: sequential, atomic, and non-atomic.

bool GB_transpose_method        // if true: use GB_builder, false: use bucket
(
    const GrB_Matrix A,         // matrix to transpose
    int *nworkspaces_bucket,    // # of slices of A for the bucket method
    int *nthreads_bucket,       // # of threads to use for the bucket method
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    int64_t anvec = (A->nvec_nonempty < 0) ? A->nvec : A->nvec_nonempty ;
    int64_t anz = GB_NNZ (A) ;
    int64_t avlen = A->vlen ;
    int64_t avdim = A->vdim ;
    int anzlog = (anz   == 0) ? 1 : (int) ceil (log2 ((double) anz)) ;
    int mlog   = (avlen == 0) ? 1 : (int) ceil (log2 ((double) avlen)) ;
    double alpha ;

    // determine # of threads for bucket method
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (anz + avlen, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // select between the atomic and non-atomic bucket method
    //--------------------------------------------------------------------------

    bool atomics ;
    if (nthreads == 1)
    { 
        // sequential bucket method, no atomics needed
        atomics = false ;
    }
    else if ((double) nthreads * (double) avlen > (double) anz)
    { 
        // non-atomic workspace is too high; use atomic method
        atomics = true ;
    }
    else
    {
        // select between atomic and non-atomic methods.  This rule is based on
        // performance on a 4-core system with 4 threads with gcc 7.5.  The icc
        // compiler has much slower atomics than gcc and so beta should likely
        // be smaller when using icc.

        int beta ;
        if (anzlog < 14)
        { 
            beta = -4 ;     // fewer than 16K entries in A
        }
        else
        { 
            switch (anzlog)
            {
                case 14: beta = -4 ; break ;        // 16K entried in A
                case 15: beta = -3 ; break ;        // 32K
                case 16: beta = -2 ; break ;        // 64K
                case 17: beta = -1 ; break ;        // 128K
                case 18: beta =  0 ; break ;        // 256K
                case 19: beta =  1 ; break ;        // 512K
                case 20: beta =  2 ; break ;        // 1M
                case 21: beta =  3 ; break ;        // 2M
                case 22: beta =  4 ; break ;        // 4M
                case 23: beta =  5 ; break ;        // 8M
                case 24: beta =  6 ; break ;        // 16M
                case 25: beta =  8 ; break ;        // 32M
                case 26: beta =  9 ; break ;        // 64M
                case 27: beta =  9 ; break ;        // 128M
                case 28: beta = 10 ; break ;        // 256M
                default: beta = 10 ; break ;        // > 256M
            }
        }

        if (anzlog - mlog <= beta)
        { 
            // use atomic method
            // anzlog - mlog is the log2 of the average row degree, rounded.
            // If the average row degree is <= 2^beta, use the atomic method.
            // That is, the atomic method works better for sparser matrices,
            // and the non-atomic works better or denser matrices.  However,
            // the threshold changes as the problem gets larger, in terms of #
            // of entries in A, when the atomic method becomes more attractive
            // relative to the non-atomic method.  The atomic has the
            // advantange of needing much less workspace, which becomes more
            // important for larger problems.
            atomics = true ;
        }
        else
        { 
            // use non-atomic method
            atomics = false ;
        }
    }

    (*nworkspaces_bucket) = (atomics) ? 1 : nthreads ;
    (*nthreads_bucket) = nthreads ;

    //--------------------------------------------------------------------------
    // select between GB_builder method and bucket method
    //--------------------------------------------------------------------------

    // As the problem gets larger, the GB_builder method gets faster relative
    // to the bucket method, in terms of the "constants" in the O(a log a) work
    // for GB_builder, or O (a+m+n) for the bucket method.  Clearly, O (a log
    // a) and O (a+m+n) do not fully model the performance of these two
    // methods.  Perhaps this is because of cache effects.  The bucket method
    // has more irregular memory accesses.  The GB_builder method uses
    // mergesort, which has good memory locality.

    if (anzlog < 14)
    { 
        alpha = 0.5 ;       // fewer than 2^14 = 16K entries
    }
    else
    { 
        switch (anzlog)
        {
            case 14: alpha = 0.6 ; break ;      // 16K entries in A
            case 15: alpha = 0.7 ; break ;      // 32K
            case 16: alpha = 1.0 ; break ;      // 64K
            case 17: alpha = 1.7 ; break ;      // 128K
            case 18: alpha = 3.0 ; break ;      // 256K
            case 19: alpha = 4.0 ; break ;      // 512K
            case 20: alpha = 6.0 ; break ;      // 1M
            case 21: alpha = 7.0 ; break ;      // 2M
            case 22: alpha = 8.0 ; break ;      // 4M
            case 23: alpha = 5.0 ; break ;      // 8M
            case 24: alpha = 5.0 ; break ;      // 16M
            case 25: alpha = 5.0 ; break ;      // 32M
            case 26: alpha = 5.0 ; break ;      // 64M
            case 27: alpha = 5.0 ; break ;      // 128M
            case 28: alpha = 5.0 ; break ;      // 256M
            default: alpha = 5.0 ; break ;      // > 256M
        }
    }

    double bucket_work  = (double) (anz + avlen + anvec) * alpha ;
    double builder_work = (log2 ((double) anz + 1) * (anz)) ;

    //--------------------------------------------------------------------------
    // select the method with the least amount of work
    //--------------------------------------------------------------------------

    return (builder_work < bucket_work) ;
}

