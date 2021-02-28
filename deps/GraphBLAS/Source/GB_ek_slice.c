//------------------------------------------------------------------------------
// GB_ek_slice: slice the entries and vectors of a matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Slice the entries of a matrix or vector into ntasks slices.

// Task t does entries pstart_slice [t] to pstart_slice [t+1]-1 and
// vectors kfirst_slice [t] to klast_slice [t].  The first and last vectors
// may be shared with prior slices and subsequent slices.

// On input, ntasks must be <= nnz (A), unless nnz (A) is zero.  In that
// case, ntasks must be 1.

#include "GB_ek_slice.h"

bool GB_ek_slice        // true if successful, false if out of memory
(
    // output:
    int64_t *GB_RESTRICT *pstart_slice_handle, // size ntasks+1
    int64_t *GB_RESTRICT *kfirst_slice_handle, // size ntasks
    int64_t *GB_RESTRICT *klast_slice_handle,  // size ntasks
    // input:
    GrB_Matrix A,                   // matrix to slice
    int ntasks                      // # of tasks
)
{

    //--------------------------------------------------------------------------
    // allocate result
    //--------------------------------------------------------------------------

    (*pstart_slice_handle) = NULL ;
    (*kfirst_slice_handle) = NULL ;
    (*klast_slice_handle ) = NULL ;

    int64_t *GB_RESTRICT pstart_slice = NULL ;
    int64_t *GB_RESTRICT kfirst_slice = NULL ;
    int64_t *GB_RESTRICT klast_slice  = NULL ;

    GB_CALLOC_MEMORY (pstart_slice, ntasks+1, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (kfirst_slice, ntasks, sizeof (int64_t)) ;
    GB_CALLOC_MEMORY (klast_slice, ntasks, sizeof (int64_t)) ;

    if (pstart_slice == NULL || kfirst_slice == NULL || klast_slice == NULL)
    { 
        GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice, ntasks) ;
        return (false) ;
    }

    (*pstart_slice_handle) = pstart_slice ;
    (*kfirst_slice_handle) = kfirst_slice ;
    (*klast_slice_handle ) = klast_slice ;

    //--------------------------------------------------------------------------
    // get A
    //--------------------------------------------------------------------------

    int64_t anvec = A->nvec ;
    int64_t anz = GB_NNZ (A) ;
    const int64_t *Ap = A->p ;

    if (anz == 0)
    { 
        // quick return for empty matrices
        ASSERT (ntasks == 1) ;
        pstart_slice [0] = 0 ;
        pstart_slice [1] = 0 ;
        kfirst_slice [0] = -1 ;
        klast_slice  [0] = -2 ;
        return (true) ;
    }

    ASSERT (ntasks <= anz) ;

    //--------------------------------------------------------------------------
    // find the first and last entries in each slice
    //--------------------------------------------------------------------------

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

    for (int taskid = 0 ; taskid < ntasks ; taskid++)
    { 

        // The slice for task taskid contains entries pfirst:plast-1 of A.
        int64_t pfirst = pstart_slice [taskid] ;
        int64_t plast  = pstart_slice [taskid+1] - 1 ;

        ASSERT (pfirst <= plast) ;

        // find the first vector of the slice for task taskid: the
        // vector that owns the entry Ai [pfirst] and Ax [pfirst].
        int64_t kfirst = GB_search_for_vector (pfirst, Ap, 0, anvec) ;

        // find the last vector of the slice for task taskid: the
        // vector that owns the entry Ai [plast] and Ax [plast].
        int64_t klast = GB_search_for_vector (plast, Ap, kfirst, anvec) ;

        kfirst_slice [taskid] = kfirst ;
        klast_slice  [taskid] = klast ;
        ASSERT (0 <= kfirst && kfirst <= klast && klast < anvec) ;
    }

    kfirst_slice [0] = 0 ;
    klast_slice  [ntasks-1] = anvec-1 ;
    return (true) ;
}

