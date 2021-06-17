//------------------------------------------------------------------------------
// GB_ek_slice: slice the entries and vectors of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Slice the entries of a matrix or vector into ntasks slices.

// The function is called GB_ek_slice because it first partitions the e entries
// into chunks of identical sizes, and then finds the first and last vector
// (k) for each chunk.

// Task t does entries pstart_slice [t] to pstart_slice [t+1]-1 and
// vectors kfirst_slice [t] to klast_slice [t].  The first and last vectors
// may be shared with prior slices and subsequent slices.

// On input, ntasks is the # of tasks requested.  On output, it may be
// modified if too large or too small.

// A can have any sparsity structure (sparse, hyper, bitmap, or full)

#include "GB_ek_slice.h"

bool GB_ek_slice        // true if successful, false if out of memory
(
    // output:
    int64_t *GB_RESTRICT *pstart_slice_handle, // size ntasks+1
    int64_t *GB_RESTRICT *kfirst_slice_handle, // size ntasks
    int64_t *GB_RESTRICT *klast_slice_handle,  // size ntasks
    // input:
    GrB_Matrix A,                   // matrix to slice
    // input/output:
    int *ntasks_handle              // # of tasks (may be modified)
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (pstart_slice_handle != NULL) ;
    ASSERT (kfirst_slice_handle != NULL) ;
    ASSERT (klast_slice_handle  != NULL) ;
    ASSERT (ntasks_handle != NULL) ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    ASSERT (GB_JUMBLED_OK (A)) ;    // pattern of A is not accessed

    int64_t anvec = A->nvec ;
    int64_t avlen = A->vlen ;
    int64_t anz = GB_NNZ_HELD (A) ;
    const int64_t *Ap = A->p ;      // NULL if bitmap or full

    // ntasks must be in the range [1,anz], inclusive, unless anz is zero.
    int ntasks = (*ntasks_handle) ;
    if (anz == 0)
    { 
        ntasks = 1 ;
    }
    else
    { 
        ntasks = GB_IMIN (ntasks, anz) ;
        ntasks = GB_IMAX (ntasks, 1) ;
    }
    (*ntasks_handle) = ntasks ;

    //--------------------------------------------------------------------------
    // allocate result
    //--------------------------------------------------------------------------

    (*pstart_slice_handle) = NULL ;
    (*kfirst_slice_handle) = NULL ;
    (*klast_slice_handle ) = NULL ;

    int64_t *GB_RESTRICT pstart_slice = GB_CALLOC (ntasks+1, int64_t) ;
    int64_t *GB_RESTRICT kfirst_slice = GB_CALLOC (ntasks  , int64_t) ;
    int64_t *GB_RESTRICT klast_slice  = GB_CALLOC (ntasks  , int64_t) ;

    if (pstart_slice == NULL || kfirst_slice == NULL || klast_slice == NULL)
    { 
        GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;
        return (false) ;
    }

    (*pstart_slice_handle) = pstart_slice ;
    (*kfirst_slice_handle) = kfirst_slice ;
    (*klast_slice_handle ) = klast_slice ;

    //--------------------------------------------------------------------------
    // quick return for empty matrices
    //--------------------------------------------------------------------------

    if (anz == 0)
    { 
        // construct a single empty task
        ASSERT (ntasks == 1) ;
        pstart_slice [0] = 0 ;
        pstart_slice [1] = 0 ;
        kfirst_slice [0] = -1 ;
        klast_slice  [0] = -2 ;
        return (true) ;
    }

    //--------------------------------------------------------------------------
    // find the first and last entries in each slice
    //--------------------------------------------------------------------------

    // FUTURE: this can be done in parallel if there are many tasks
    GB_eslice (pstart_slice, anz, ntasks) ;

    //--------------------------------------------------------------------------
    // find the first and last vectors in each slice
    //--------------------------------------------------------------------------

    // The first vector of the slice is the kth vector of A if
    // pstart_slice [taskid] is in the range Ap [k]...A[k+1]-1, and this
    // is vector is k = kfirst_slice [taskid].

    // The last vector of the slice is the kth vector of A if
    // pstart_slice [taskid+1]-1 is in the range Ap [k]...A[k+1]-1, and this
    // is vector is k = klast_slice [taskid].

    // FUTURE: this can be done in parallel if there are many tasks
    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    { 

        // The slice for task taskid contains entries pfirst:plast-1 of A.
        int64_t pfirst = pstart_slice [taskid] ;
        int64_t plast  = pstart_slice [taskid+1] - 1 ;

        ASSERT (pfirst <= plast) ;

        // find the first vector of the slice for task taskid: the
        // vector that owns the entry Ai [pfirst] and Ax [pfirst].
        int64_t kfirst = GB_search_for_vector (pfirst, Ap, 0, anvec, avlen) ;

        // find the last vector of the slice for task taskid: the
        // vector that owns the entry Ai [plast] and Ax [plast].
        int64_t klast = GB_search_for_vector (plast, Ap, kfirst, anvec, avlen) ;

        kfirst_slice [taskid] = kfirst ;
        klast_slice  [taskid] = klast ;
        ASSERT (0 <= kfirst && kfirst <= klast && klast < anvec) ;
    }

    kfirst_slice [0] = 0 ;
    klast_slice  [ntasks-1] = anvec-1 ;
    return (true) ;
}

