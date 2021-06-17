//------------------------------------------------------------------------------
// GB_builder: build a matrix from tuples
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// CALLED BY: GB_build, GB_Matrix_wait, and GB_transpose
// CALLS:     Generated/GB_red_build__* workers

// This function is called by GB_build to build a matrix T for GrB_Matrix_build
// or GrB_Vector_build, by GB_Matrix_wait to build a matrix T from the list of
// pending tuples, and by GB_transpose to transpose a matrix or vector.
// Duplicates can appear if called by GB_build or GB_Matrix_wait, but not
// GB_transpose.

// The indices are provided either as (I_input,J_input) or (I_work,J_work), not
// both.  The values are provided as S_input or S_work, not both.  On return,
// the *work arrays are either transplanted into T, or freed, since they are
// temporary workspaces.

// The work is done in major 5 Steps, some of which can be skipped, depending
// on how the tuples are provided (*_work or *_input), and whether or not they
// are sorted, or have duplicates.  If vdim <= 1, some work is skipped (for
// GrB_Vectors, and single-vector GrB_Matrices).  Let e be the of tuples on
// input.  Let p be the # of threads used.

// STEP 1: copy user input.  O(e/p) read/write per thread, or skipped.

// STEP 2: sort the tuples.  Time: O((e log e)/p), read/write, or skipped if
//         the tuples are already sorted.

// STEP 3: count vectors and duplicates.  O(e/p) reads, per thread, if no
//         duplicates, or skipped if already done.  O(e/p) read/writes
//         per thread if duplicates appear.

// STEP 4: construct T->h and T->p.  O(e/p) reads per thread, or skipped if
//         T is a vector.

// STEP 5: assemble the tuples.  O(e/p) read/writes per thread, or O(1) if the
//         values can be transplanted into T as-is.

// For GrB_Matrix_build:  If the input (I_input, J_input, S_input) is already
// sorted with no duplicates, and no typecasting needs to be done, then Step 1
// still must be done (each thread does O(e/p) reads of (I_input,J_input) and
// writes to I_work), but Step 1 also does the work for Step 3.  Step 2 and 3
// are skipped.  Step 4 does O(e/p) reads per thread (J_input only).  Then
// I_work is transplanted into T->i.  Step 5 does O(e/p) read/writes per thread
// to copy S into T->x.

// For GrB_Vector_build: as GrB_Matrix_build, Step 1 does O(e/p) read/writes
// per thread.  The input is always a vector, so vdim == 1 always holds.  Step
// 2 is skipped if the indices are already sorted, and Step 3 does no work at
// all unless duplicates appear.  Step 4 takes no time, for any vector. Step 5
// does O(e/p) reads/writes per thread.

// For GB_Matrix_wait:  the pending tuples are provided as I_work, J_work, and
// S_work, so Step 1 is skipped (no need to check for invalid indices).  The
// input J_work may be null (vdim can be anything, since GB_Matrix_wait is used
// for both vectors and matrices).  The tuples might be in sorted order
// already, which is known precisely known from A->Pending->sorted.  Step 2
// does O((e log e)/p) work to sort the tuples.  Duplicates may appear, and
// out-of-order tuples are likely.  Step 3 does O(e/p) read/writes.  Step 4
// does O(e/p) reads per thread of (I_work,J_work), or just I_work.  Step 5
// does O(e/p) read/writes per thread, or O(1) time if S_work can be
// transplanted into T->x.

// For GB_transpose: uses I_work, J_work, and either S_input (if no op applied
// to the values) or S_work (if an op was applied to the A->x values).  This is
// only done for matrices, not vectors, so vdim > 1 will always hold.  The
// indices are valid so Step 1 is skipped.  The tuples are not sorted, so Step
// 2 takes O((e log e)/p) time to do the sort.  There are no duplicates, so
// Step 3 only does O(e/p) reads of J_work to count the vectors in each slice.
// Step 4 only does O(e/p) reads of J_work to compute T->h and T->p.  Step 5
// does O(e/p) read/writes per thread, but it uses the simpler case in
// GB_reduce_build_template since no duplicates can appear.  It is unlikely
// able to transplant S_work into T->x since the input will almost always be
// unsorted.

// For BITMAP case: this method always returns T as hypersparse, and has no
// matrix inputs.   If the final C should become full or bitmap, that
// conversion is done by GB_transplant_conform.

#include "GB_build.h"
#include "GB_sort.h"
#include "GB_binop.h"
#ifndef GBCOMPACT
#include "GB_red__include.h"
#endif

#define GB_I_WORK(t) (((t) < 0) ? -1 : I_work [t])
#define GB_J_WORK(t) (((t) < 0) ? -1 : ((J_work == NULL) ? 0 : J_work [t]))
#define GB_K_WORK(t) (((t) < 0) ? -1 : ((K_work == NULL) ? t : K_work [t]))

#define GB_FREE_WORK                                                \
{                                                                   \
    GB_FREE (tstart_slice) ;        \
    GB_FREE (tnvec_slice) ;         \
    GB_FREE (tnz_slice) ;           \
    GB_FREE (kbad) ;                \
    GB_FREE (ilast_slice) ;         \
    GB_FREE (*I_work_handle) ;      \
    GB_FREE (*J_work_handle) ;      \
    GB_FREE (*S_work_handle) ;      \
    GB_FREE (K_work) ;              \
}

//------------------------------------------------------------------------------
// GB_builder
//------------------------------------------------------------------------------

GrB_Info GB_builder                 // build a matrix from tuples
(
    GrB_Matrix *Thandle,            // matrix T to build
    const GrB_Type ttype,           // type of output matrix T
    const int64_t vlen,             // length of each vector of T
    const int64_t vdim,             // number of vectors in T
    const bool is_csc,              // true if T is CSC, false if CSR
    int64_t **I_work_handle,        // for (i,k) or (j,i,k) tuples
    int64_t **J_work_handle,        // for (j,i,k) tuples
    GB_void **S_work_handle,        // array of values of tuples, size ijslen
    bool known_sorted,              // true if tuples known to be sorted
    bool known_no_duplicates,       // true if tuples known to not have dupl
    int64_t ijslen,                 // size of I_work and J_work arrays
    const bool is_matrix,           // true if T a GrB_Matrix, false if vector
    const int64_t *GB_RESTRICT I_input,// original indices, size nvals
    const int64_t *GB_RESTRICT J_input,// original indices, size nvals
    const GB_void *GB_RESTRICT S_input,// array of values of tuples, size nvals
    const int64_t nvals,            // number of tuples, and size of K_work
    const GrB_BinaryOp dup,         // binary function to assemble duplicates,
                                    // if NULL use the SECOND operator to
                                    // keep the most recent duplicate.
    const GB_Type_code scode,       // GB_Type_code of S_work or S_input array
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Thandle != NULL) ;
    ASSERT (nvals >= 0) ;
    ASSERT (scode <= GB_UDT_code) ;
    ASSERT_TYPE_OK (ttype, "ttype for builder", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (dup, "dup for builder", GB0) ;
    ASSERT (I_work_handle != NULL) ;
    ASSERT (J_work_handle != NULL) ;
    ASSERT (S_work_handle != NULL) ;
    ASSERT (!GB_OP_IS_POSITIONAL (dup)) ;

    //--------------------------------------------------------------------------
    // get S
    //--------------------------------------------------------------------------

    GB_void *GB_RESTRICT S_work = (*S_work_handle) ;

    const GB_void *GB_RESTRICT S = (S_work == NULL) ? S_input : S_work ;
    size_t tsize = ttype->size ;
    size_t ssize = GB_code_size (scode, tsize) ;
    ASSERT (GB_IMPLIES (nvals > 0, S != NULL)) ;

    //==========================================================================
    // symbolic phase of the build =============================================
    //==========================================================================

    // The symbolic phase sorts the tuples and finds any duplicates.  The
    // output matrix T is constructed (not including T->i and T->x), and T->h
    // and T->p are computed.  Then I_work is transplanted into T->i, or T->i is
    // allocated.  T->x is then allocated.  It is not computed until the
    // numeric phase.

    // When this function returns, I_work is either freed or transplanted into
    // T->i.  J_work is freed, and the I_work and J_work pointers (in the
    // caller) are set to NULL by setting their handles to NULL.  Note that
    // J_work may already be NULL on input, if T has one or zero vectors
    // (J_work_handle is always non-NULL however).

    GrB_Info info ;
    GrB_Matrix T = NULL ;
    (*Thandle) = NULL ;
    int64_t *GB_RESTRICT I_work = (*I_work_handle) ;
    int64_t *GB_RESTRICT J_work = (*J_work_handle) ;
    int64_t *GB_RESTRICT K_work = NULL ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (nvals, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // allocate workspace
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT tstart_slice = NULL ;     // size nthreads+1
    int64_t *GB_RESTRICT tnvec_slice = NULL ;      // size nthreads+1
    int64_t *GB_RESTRICT tnz_slice = NULL ;        // size nthreads+1
    int64_t *GB_RESTRICT kbad = NULL ;             // size nthreads
    int64_t *GB_RESTRICT ilast_slice = NULL ;      // size [nthreads]

    tstart_slice = GB_CALLOC (nthreads+1, int64_t) ;
    tnvec_slice  = GB_CALLOC (nthreads+1, int64_t) ;
    tnz_slice    = GB_CALLOC (nthreads+1, int64_t) ;
    kbad         = GB_CALLOC (nthreads,   int64_t) ;
    ilast_slice  = GB_CALLOC (nthreads,   int64_t) ;

    if (tstart_slice == NULL || tnvec_slice == NULL || tnz_slice == NULL ||
        kbad == NULL || ilast_slice == NULL)
    { 
        // out of memory
        GB_FREE_WORK ;
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // partition the tuples for the threads
    //--------------------------------------------------------------------------

    // Thread tid handles tuples tstart_slice [tid] to tstart_slice [tid+1]-1.
    // Each thread handles about the same number of tuples.  This partition
    // depends only on nvals.

    tstart_slice [0] = 0 ;
    for (int tid = 1 ; tid < nthreads ; tid++)
    { 
        tstart_slice [tid] = GB_PART (tid, nvals, nthreads) ;
    }
    tstart_slice [nthreads] = nvals ;

    // tstart_slice [tid]: first tuple in slice tid
    // tnvec_slice [tid]: # of vectors that start in a slice.  If a vector
    //                    starts in one slice and ends in another, it is
    //                    counted as being in the first slice.
    // tnz_slice   [tid]: # of entries in a slice after removing duplicates

    // sentinel values for the final cumulative sum
    tnvec_slice [nthreads] = 0 ;
    tnz_slice   [nthreads] = 0 ;

    // this becomes true if the first pass computes tnvec_slice and tnz_slice,
    // and if the (I_input,J_input) tuples were found to be already sorted with
    // no duplicates present.
    bool tnvec_and_tnz_slice_computed = false ;

    //--------------------------------------------------------------------------
    // STEP 1: copy user input and check if valid
    //--------------------------------------------------------------------------

    // If the indices are provided by (I_input,J_input), then import them into
    // (I_work,J_work) and check if they are valid, and sorted.   If the input
    // happens to be already sorted, then duplicates are detected and the # of
    // vectors in each slice is counted.

    if (I_work == NULL)
    {

        //----------------------------------------------------------------------
        // allocate I_work
        //----------------------------------------------------------------------

        // allocate workspace to load and sort the index tuples:

        // vdim <= 1: I_work and K_work for (i,k) tuples, where i = I_input [k]

        // vdim > 1: also J_work for (j,i,k) tuples where i = I_input [k] and
        // j = J_input [k].  If the tuples are found to be already sorted on
        // input, then J_work is not allocated, and J_input is used instead.

        // The k value in the tuple gives the position in the original set of
        // tuples: I_input [k] and S [k] when vdim <= 1, and also J_input [k]
        // for matrices with vdim > 1.

        // The workspace I_work and J_work are allocated here but freed (or
        // transplanted) inside GB_builder.  K_work is allocated, used, and
        // freed in GB_builder.

        ASSERT (J_work == NULL) ;
        I_work = GB_MALLOC (nvals, int64_t) ;
        (*I_work_handle) = I_work ;
        ijslen = nvals ;
        if (I_work == NULL)
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        //----------------------------------------------------------------------
        // create the tuples to sort, and check for any invalid indices
        //----------------------------------------------------------------------

        known_sorted = true ;
        bool no_duplicates_found = true ;

        if (nvals == 0)
        { 

            // nothing to do

        }
        else if (is_matrix)
        {

            //------------------------------------------------------------------
            // C is a matrix; check both I_input and J_input
            //------------------------------------------------------------------

            ASSERT (J_input != NULL) ;
            ASSERT (I_work != NULL) ;
            ASSERT (vdim >= 0) ;
            ASSERT (I_input != NULL) ;

            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static) \
                reduction(&&:known_sorted) reduction(&&:no_duplicates_found)
            for (tid = 0 ; tid < nthreads ; tid++)
            {

                kbad [tid] = -1 ;
                int64_t my_tnvec = 0 ;
                int64_t kstart   = tstart_slice [tid] ;
                int64_t kend     = tstart_slice [tid+1] ;
                int64_t ilast = (kstart == 0) ? -1 : I_input [kstart-1] ;
                int64_t jlast = (kstart == 0) ? -1 : J_input [kstart-1] ;

                for (int64_t k = kstart ; k < kend ; k++)
                {
                    // get k-th index from user input: (i,j)
                    int64_t i = I_input [k] ;
                    int64_t j = J_input [k] ;

                    if (i < 0 || i >= vlen || j < 0 || j >= vdim)
                    { 
                        // halt if out of bounds
                        kbad [tid] = k ;
                        break ;
                    }

                    // check if the tuples are already sorted
                    known_sorted = known_sorted &&
                        ((jlast < j) || (jlast == j && ilast <= i)) ;

                    // check if this entry is a duplicate of the one before it
                    no_duplicates_found = no_duplicates_found &&
                        (!(jlast == j && ilast == i)) ;

                    // copy the tuple into I_work.  J_work is done later.
                    I_work [k] = i ;

                    if (j > jlast)
                    { 
                        // vector j starts in this slice (but this is
                        // valid only if J_input is sorted on input)
                        my_tnvec++ ;
                    }

                    // log the last index seen
                    ilast = i ; jlast = j ;
                }

                // these are valid only if I_input and J_input are sorted on
                // input, with no duplicates present.
                tnvec_slice [tid] = my_tnvec ;
                tnz_slice   [tid] = kend - kstart ;

            }

            // collect the report from each thread
            for (int tid = 0 ; tid < nthreads ; tid++)
            {
                if (kbad [tid] >= 0)
                { 
                    // invalid index
                    int64_t i = I_input [kbad [tid]] ;
                    int64_t j = J_input [kbad [tid]] ;
                    int64_t row = is_csc ? i : j ;
                    int64_t col = is_csc ? j : i ;
                    int64_t nrows = is_csc ? vlen : vdim ;
                    int64_t ncols = is_csc ? vdim : vlen ;
                    GB_FREE_WORK ;
                    GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS,
                        "index (" GBd "," GBd ") out of bounds,"
                        " must be < (" GBd ", " GBd ")",
                        row, col, nrows, ncols) ;
                }
            }

            // if the tuples were found to be already in sorted order, and if
            // no duplicates were found, then tnvec_slice and tnz_slice are now
            // valid, Otherwise, they can only be computed after sorting.
            tnvec_and_tnz_slice_computed = known_sorted && no_duplicates_found ;

            //------------------------------------------------------------------
            // allocate J_work, if needed
            //------------------------------------------------------------------

            if (vdim > 1 && !known_sorted)
            {
                // copy J_input into J_work, so the tuples can be sorted
                J_work = GB_MALLOC (nvals, int64_t) ;
                (*J_work_handle) = J_work ;
                if (J_work == NULL)
                { 
                    // out of memory
                    GB_FREE_WORK ;
                    return (GrB_OUT_OF_MEMORY) ;
                }
                GB_memcpy (J_work, J_input, nvals * sizeof (int64_t), nthreads);
            }
            else
            { 
                // J_work is a shallow copy of J_input.  The pointer is not
                // copied into (*J_work_handle), so it will not be freed.
                // J_input is not modified, even though it is typecast to the
                // int64_t *J_work, since J_work is not modified in this case.
                J_work = (int64_t *) J_input ;
            }

        }
        else
        {

            //------------------------------------------------------------------
            // C is a typecasted GrB_Vector; check only I_input
            //------------------------------------------------------------------

            ASSERT (I_input != NULL) ;
            ASSERT (J_input == NULL) ;
            ASSERT (vdim == 1) ;

            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static) \
                reduction(&&:known_sorted) reduction(&&:no_duplicates_found)
            for (tid = 0 ; tid < nthreads ; tid++)
            {

                kbad [tid] = -1 ;
                int64_t kstart   = tstart_slice [tid] ;
                int64_t kend     = tstart_slice [tid+1] ;
                int64_t ilast = (kstart == 0) ? -1 : I_input [kstart-1] ;

                for (int64_t k = kstart ; k < kend ; k++)
                {
                    // get k-th index from user input: (i)
                    int64_t i = I_input [k] ;

                    if (i < 0 || i >= vlen)
                    { 
                        // halt if out of bounds
                        kbad [tid] = k ;
                        break ;
                    }

                    // check if the tuples are already sorted
                    known_sorted = known_sorted && (ilast <= i) ;

                    // check if this entry is a duplicate of the one before it
                    no_duplicates_found = no_duplicates_found &&
                        (!(ilast == i)) ;

                    // copy the tuple into the work arrays to be sorted
                    I_work [k] = i ;

                    // log the last index seen
                    ilast = i ;
                }
            }

            // collect the report from each thread
            for (int tid = 0 ; tid < nthreads ; tid++)
            {
                if (kbad [tid] >= 0)
                { 
                    // invalid index
                    int64_t i = I_input [kbad [tid]] ;
                    GB_FREE_WORK ;
                    GB_ERROR (GrB_INDEX_OUT_OF_BOUNDS,
                        "index (" GBd ") out of bounds, must be < (" GBd ")",
                        i, vlen) ;
                }
            }
        }

        //----------------------------------------------------------------------
        // determine if duplicates are possible
        //----------------------------------------------------------------------

        // The input is now known to be sorted, or not.  If it is sorted, and
        // if no duplicates were found, then it is known to have no duplicates.
        // Otherwise, duplicates might appear, but a sort is required first to
        // check for duplicates.

        known_no_duplicates = known_sorted && no_duplicates_found ;
    }

    //--------------------------------------------------------------------------
    // STEP 2: sort the tuples in ascending order
    //--------------------------------------------------------------------------

    // If the tuples are known to already be sorted, Step 2 is skipped.  In
    // that case, K_work is NULL (not allocated), which implicitly means that
    // K_work [k] = k for all k = 0:nvals-1.

    if (!known_sorted)
    {

        // create the k part of each tuple
        K_work = GB_MALLOC (nvals, int64_t) ;
        if (K_work == NULL)
        { 
            // out of memory
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        // The k part of each tuple (i,k) or (j,i,k) records the original
        // position of the tuple in the input list.  This allows an unstable
        // sorting algorith to be used.  Since k is unique, it forces the
        // result of the sort to be stable regardless of whether or not the
        // sorting algorithm is stable.  It also keeps track of where the
        // numerical value of the tuple can be found; it is in S[k] for the
        // tuple (i,k) or (j,i,k), regardless of where the tuple appears in the
        // list after it is sorted.
        int64_t k ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (k = 0 ; k < nvals ; k++)
        { 
            K_work [k] = k ;
        }

        // sort all the tuples
        if (vdim > 1)
        {
            // sort a set of (j,i,k) tuples
            GB_msort_3b (J_work, I_work, K_work, nvals, nthreads) ;
        }
        else
        {
            // sort a set of (i,k) tuples
            GB_msort_2b (I_work, K_work, nvals, nthreads) ;
        }
    }

    //--------------------------------------------------------------------------
    // STEP 3: count vectors and duplicates in each slice
    //--------------------------------------------------------------------------

    // Duplicates are located, counted and their indices negated.  The # of
    // vectors in each slice is counted.  If the indices are known to not have
    // duplicates, then only the vectors are counted.  Counting the # of
    // vectors is skipped if already done by Step 1.

    if (known_no_duplicates)
    {

        //----------------------------------------------------------------------
        // no duplicates: just count # vectors in each slice
        //----------------------------------------------------------------------

        // This is much faster, particularly if the # of vectors in each slice
        // has already been computed.

        #ifdef GB_DEBUG
        {
            // assert that there are no duplicates
            int64_t ilast = -1, jlast = -1 ;
            for (int64_t t = 0 ; t < nvals ; t++)
            {
                int64_t i = GB_I_WORK (t), j = GB_J_WORK (t) ;
                bool is_duplicate = (i == ilast && j == jlast) ;
                ASSERT (!is_duplicate) ;
                ilast = i ; jlast = j ;
            }
        }
        #endif

        if (vdim <= 1)
        {

            // all tuples appear in at most one vector, and there are no
            // duplicates, so there is no need to scan I_work or J_work.

            for (int tid = 0 ; tid < nthreads ; tid++)
            { 
                int64_t tstart = tstart_slice [tid] ;
                int64_t tend   = tstart_slice [tid+1] ;
                tnvec_slice [tid] = 0 ;
                tnz_slice   [tid] = tend - tstart ;
            }
            tnvec_slice [0] = (nvals == 0) ? 0 : 1 ;

        }
        else
        {

            // count the # of unique vector indices in J_work.  No need to scan
            // I_work since there are no duplicates to be found.  Also no need
            // to compute them if already found in Step 1. 

            if (!tnvec_and_tnz_slice_computed)
            {

                int tid ;
                #pragma omp parallel for num_threads(nthreads) schedule(static)
                for (tid = 0 ; tid < nthreads ; tid++)
                {
                    int64_t my_tnvec = 0 ;
                    int64_t tstart = tstart_slice [tid] ;
                    int64_t tend   = tstart_slice [tid+1] ;
                    int64_t jlast  = GB_J_WORK (tstart-1) ;

                    for (int64_t t = tstart ; t < tend ; t++)
                    {
                        // get the t-th tuple
                        int64_t j = J_work [t] ;
                        if (j > jlast)
                        { 
                            // vector j starts in this slice
                            my_tnvec++ ;
                            jlast = j ;
                        }
                    }

                    tnvec_slice [tid] = my_tnvec ;
                    tnz_slice   [tid] = tend - tstart ;
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // look for duplicates and count # vectors in each slice
        //----------------------------------------------------------------------

        for (int tid = 0 ; tid < nthreads ; tid++)
        { 
            int64_t tstart = tstart_slice [tid] ;
            ilast_slice [tid] = GB_I_WORK (tstart-1) ;
        }

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {

            int64_t my_tnvec = 0 ;
            int64_t my_ndupl = 0 ;
            int64_t tstart   = tstart_slice [tid] ;
            int64_t tend     = tstart_slice [tid+1] ;
            int64_t ilast    = ilast_slice [tid] ;
            int64_t jlast    = GB_J_WORK (tstart-1) ;

            for (int64_t t = tstart ; t < tend ; t++)
            {
                // get the t-th tuple
                int64_t i = I_work [t] ;
                int64_t j = GB_J_WORK (t) ;

                // tuples are now sorted but there may be duplicates
                ASSERT ((jlast < j) || (jlast == j && ilast <= i)) ;

                // check if (j,i,k) is a duplicate
                if (i == ilast && j == jlast)
                { 
                    // flag the tuple as a duplicate
                    I_work [t] = -1 ;
                    my_ndupl++ ;
                    // the sort places earlier duplicate tuples (with smaller
                    // k) after later ones (with larger k).
                    ASSERT (GB_K_WORK (t-1) < GB_K_WORK (t)) ;
                }
                else
                {
                    // this is a new tuple
                    if (j > jlast)
                    { 
                        // vector j starts in this slice
                        my_tnvec++ ;
                        jlast = j ;
                    }
                    ilast = i ;
                }
            }
            tnvec_slice [tid] = my_tnvec ;
            tnz_slice   [tid] = (tend - tstart) - my_ndupl ;
        }
    }

    //--------------------------------------------------------------------------
    // find total # of vectors and duplicates in all tuples
    //--------------------------------------------------------------------------

    // Replace tnvec_slice with its cumulative sum, after which each slice tid
    // will be responsible for the # vectors in T that range from tnvec_slice
    // [tid] to tnvec_slice [tid+1]-1.
    GB_cumsum (tnvec_slice, nthreads, NULL, 1) ;
    int64_t tnvec = tnvec_slice [nthreads] ;

    // Replace tnz_slice with its cumulative sum
    GB_cumsum (tnz_slice, nthreads, NULL, 1) ;

    // find the total # of final entries, after assembling duplicates
    int64_t tnz = tnz_slice [nthreads] ;
    int64_t ndupl = nvals - tnz ;

    //--------------------------------------------------------------------------
    // allocate T; always hypersparse
    //--------------------------------------------------------------------------

    // allocate T; allocate T->p and T->h but do not initialize them.
    // T is always hypersparse.
    info = GB_new (&T, // always hyper (even vectors), new header
        ttype, vlen, vdim, GB_Ap_malloc, is_csc,
        GxB_HYPERSPARSE, GB_ALWAYS_HYPER, tnvec, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_WORK ;
        return (info) ;
    }

    ASSERT (T->p != NULL) ;
    ASSERT (T->h != NULL) ;
    ASSERT (T->nzmax == 0) ;        // T->i and T->x not yet allocated
    ASSERT (T->b == NULL) ;
    ASSERT (T->i == NULL) ;
    ASSERT (T->x == NULL) ;

    (*Thandle) = T ;

    //--------------------------------------------------------------------------
    // STEP 4: construct the vector pointers and hyperlist for T
    //--------------------------------------------------------------------------

    // Step 4 scans the J_work indices and constructs T->h and T->p.

    int64_t *GB_RESTRICT Th = T->h ;
    int64_t *GB_RESTRICT Tp = T->p ;

    if (vdim <= 1)
    {

        //----------------------------------------------------------------------
        // special case for vectors
        //----------------------------------------------------------------------

        ASSERT (tnvec == 0 || tnvec == 1) ;
        if (tnvec > 0)
        { 
            Th [0] = 0 ;
            Tp [0] = 0 ;
        }

    }
    else if (ndupl == 0)
    {

        //----------------------------------------------------------------------
        // no duplicates appear
        //----------------------------------------------------------------------

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {

            int64_t my_tnvec = tnvec_slice [tid] ;
            int64_t tstart   = tstart_slice [tid] ;
            int64_t tend     = tstart_slice [tid+1] ;
            int64_t jlast    = GB_J_WORK (tstart-1) ;

            for (int64_t t = tstart ; t < tend ; t++)
            {
                // get the t-th tuple
                int64_t j = GB_J_WORK (t) ;
                if (j > jlast)
                { 
                    // vector j starts in this slice
                    Th [my_tnvec] = j ;
                    Tp [my_tnvec] = t ;
                    my_tnvec++ ;
                    jlast = j ;
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // it is known that at least one duplicate appears
        //----------------------------------------------------------------------

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {

            int64_t my_tnz   = tnz_slice [tid] ;
            int64_t my_tnvec = tnvec_slice [tid] ;
            int64_t tstart   = tstart_slice [tid] ;
            int64_t tend     = tstart_slice [tid+1] ;
            int64_t jlast    = GB_J_WORK (tstart-1) ;

            for (int64_t t = tstart ; t < tend ; t++)
            {
                // get the t-th tuple
                int64_t i = I_work [t] ;
                int64_t j = GB_J_WORK (t) ;
                if (i >= 0)
                {
                    // this is a new tuple
                    if (j > jlast)
                    { 
                        // vector j starts in this slice 
                        Th [my_tnvec] = j ;
                        Tp [my_tnvec] = my_tnz ;
                        my_tnvec++ ;
                        jlast = j ;
                    }
                    my_tnz++ ;
                }
            }
        }
    }

    // log the end of the last vector
    T->nvec_nonempty = tnvec ;
    T->nvec = tnvec ;
    Tp [tnvec] = tnz ;
    ASSERT (T->nvec == T->plen) ;
    T->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // free J_work if it exists
    //--------------------------------------------------------------------------

    ASSERT (J_work_handle != NULL) ;
    GB_FREE (*J_work_handle) ;
    J_work = NULL ;

    //--------------------------------------------------------------------------
    // allocate T->i
    //--------------------------------------------------------------------------

    T->nzmax = GB_IMAX (tnz, 1) ;

    if (ndupl == 0)
    {
        // shrink I_work from size ijslen to size T->nzmax
        if (T->nzmax < ijslen)
        { 
            // this cannot fail since the size is shrinking.
            bool ok ;
            GB_REALLOC (I_work, T->nzmax, ijslen, int64_t, &ok) ;
            ASSERT (ok) ;
        }
        // transplant I_work into T->i
        T->i = I_work ;
        I_work = NULL ;
        (*I_work_handle) = NULL ;
    }
    else
    {
        // duplicates exist, so allocate a new T->i.  I_work must be freed later
        T->i = GB_MALLOC (tnz, int64_t) ;
        if (T->i == NULL)
        { 
            // out of memory
            GB_Matrix_free (Thandle) ;
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }
    }

    int64_t *GB_RESTRICT Ti = T->i ;

    //==========================================================================
    // numerical phase of the build: assemble any duplicates
    //==========================================================================

    // The tuples have been sorted.  Assemble any duplicates with a switch
    // factory of built-in workers, or four generic workers.  The vector
    // pointers T->p and hyperlist T->h (if hypersparse) have already been
    // computed.

    // If there are no duplicates, T->i holds the row indices of the tuple.
    // Otherwise, the row indices are still in I_work.  K_work holds the
    // positions of each tuple in the array S.  The tuples are sorted so that
    // duplicates are adjacent to each other and they appear in the order they
    // appeared in the original tuples.  This method assembles the duplicates
    // and computes T->i and T->x from I_work, K_work, and S.  into T, becoming
    // T->i.  If no duplicates appear, T->i is already computed, and S just
    // needs to be copied and permuted into T->x.

    // The (i,k,S[k]) tuples are held in two integer arrays: (1) I_work or T->i,
    // and (2) K_work, and an array S of numerical values.  S has not been
    // sorted, nor even accessed yet.  It is identical to the original unsorted
    // tuples.  The (i,k,S[k]) tuple holds the row index i, the position k, and
    // the value S [k].  This entry becomes T(i,j) = S [k] in the matrix T, and
    // duplicates (if any) are assembled via the dup operator.

    //--------------------------------------------------------------------------
    // get opcodes and check types
    //--------------------------------------------------------------------------

    // With GB_build, there can be 1 to 2 different types.
    //      T->type is identical to the types of x,y,z for z=dup(x,y).
    //      dup is never NULL and all its three types are the same
    //      The type of S (scode) can different but must be compatible
    //          with T->type

    // With GB_Matrix_wait, there can be 1 to 5 different types:
    //      The pending tuples are in S, of type scode which must be
    //          compatible with dup->ytype and T->type
    //      z = dup (x,y): can be NULL or have 1 to 3 different types
    //      T->type: must be compatible with all above types.
    //      dup may be NULL, in which case it is assumed be the implicit SECOND
    //          operator, with all three types equal to T->type

    GrB_Type xtype, ytype, ztype ;
    GxB_binary_function fdup ;
    #ifndef GBCOMPACT
    GB_Opcode opcode ;
    #endif

    GB_Type_code tcode = ttype->code ;
    bool op_2nd ;

    ASSERT_TYPE_OK (ttype, "ttype for build_factory", GB0) ;

    if (dup == NULL)
    { 

        //----------------------------------------------------------------------
        // dup is the implicit SECOND operator
        //----------------------------------------------------------------------

        // z = SECOND (x,y) where all three types are the same as ttype
        // T(i,j) = (ttype) S(k) will be done for all tuples.

        #ifndef GBCOMPACT
        opcode = GB_SECOND_opcode ;
        #endif
        ASSERT (GB_op_is_second (dup, ttype)) ;
        xtype = ttype ;
        ytype = ttype ;
        ztype = ttype ;
        fdup = NULL ;
        op_2nd = true ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // dup is an explicit operator
        //----------------------------------------------------------------------

        // T(i,j) = (ttype) S[k] will be done for the first tuple.
        // for subsequent tuples: T(i,j) += S[k], via the dup operator and
        // typecasting:
        //
        //      y = (dup->ytype) S[k]
        //      x = (dup->xtype) T(i,j)
        //      z = (dup->ztype) dup (x,y)
        //      T(i,j) = (ttype) z

        ASSERT_BINARYOP_OK (dup, "dup for build_factory", GB0) ;
        #ifndef GBCOMPACT
        opcode = dup->opcode ;
        #endif
        xtype = dup->xtype ;
        ytype = dup->ytype ;
        ztype = dup->ztype ;
        fdup = dup->function ;
        op_2nd = GB_op_is_second (dup, ttype) ;
    }

    //--------------------------------------------------------------------------
    // get the sizes and codes of each type
    //--------------------------------------------------------------------------

    GB_Type_code zcode = ztype->code ;
    GB_Type_code xcode = xtype->code ;
    GB_Type_code ycode = ytype->code ;

    ASSERT (GB_code_compatible (tcode, scode)) ;    // T(i,j) = (ttype) S
    ASSERT (GB_code_compatible (ycode, scode)) ;    // y = (ytype) S
    ASSERT (GB_Type_compatible (xtype, ttype)) ;    // x = (xtype) T(i,j)
    ASSERT (GB_Type_compatible (ttype, ztype)) ;    // T(i,j) = (ttype) z

    size_t zsize = ztype->size ;
    size_t xsize = xtype->size ;
    size_t ysize = ytype->size ;

    // no typecasting if all 5 types are the same
    bool nocasting = (tcode == scode) &&
        (ttype == xtype) && (ttype == ytype) && (ttype == ztype) ;

    //--------------------------------------------------------------------------
    // STEP 5: assemble the tuples
    //--------------------------------------------------------------------------

    bool copy_S_into_T = (nocasting && known_sorted && ndupl == 0) ;

    if (copy_S_into_T && S_work != NULL)
    { 

        //----------------------------------------------------------------------
        // transplant S_work into T->x
        //----------------------------------------------------------------------

        // No typecasting is needed, the tuples were originally in sorted
        // order, and no duplicates appear.  All that is required is to copy S
        // into Tx.  S can be directly transplanted into T->x since S is
        // provided as S_work.  GB_builder must either transplant or free
        // S_work.  The transplant can be used by GB_Matrix_wait (whenever the
        // tuples are already sorted, with no duplicates, and no typecasting is
        // needed, since S_work is always A->Pending->x).  This transplant can
        // rarely be used for GB_transpose, in the case when op is NULL and the
        // transposed tuples happen to be sorted (which is unlikely).

        T->x = S_work ;
        S_work = NULL ;
        (*S_work_handle) = NULL ;

    }
    else
    {

        //----------------------------------------------------------------------
        // allocate T->x
        //----------------------------------------------------------------------

        T->x = GB_MALLOC (tnz * ttype->size, GB_void) ;
        if (T->x == NULL)
        { 
            // out of memory
            GB_Matrix_free (Thandle) ;
            GB_FREE_WORK ;
            return (GrB_OUT_OF_MEMORY) ;
        }

        GB_void *GB_RESTRICT Tx = (GB_void *) T->x ;

        ASSERT (GB_IMPLIES (nvals > 0, S != NULL)) ;

        if (nvals == 0)
        { 

            // nothing to do

        }
        else if (copy_S_into_T)
        { 

            //------------------------------------------------------------------
            // copy S into T->x
            //------------------------------------------------------------------

            // No typecasting is needed, the tuples were originally in sorted
            // order, and no duplicates appear.  All that is required is to
            // copy S into Tx.  S cannot be transplanted into T->x since
            // S_work is NULL and S_input cannot be modified by GB_builder.

            GBURBLE ("(build:memcpy) ") ;
            ASSERT (S_work == NULL) ;
            ASSERT (S == S_input) ;
            GB_memcpy (Tx, S, nvals * tsize, nthreads) ;

        }
        else if (nocasting)
        { 

            //------------------------------------------------------------------
            // assemble the values, S, into T, no typecasting needed
            //------------------------------------------------------------------

            // S (either S_work or S_input) must be permuted and copied into
            // T->x, since the tuples had to be sorted, or duplicates appear.
            // Any duplicates are now assembled.

            // There are 44 common cases of this function for built-in types
            // and 8 associative operators: MIN, MAX, PLUS, TIMES for 10 types
            // (all but boolean; and OR, AND, XOR, and EQ for boolean.

            // In addition, the FIRST and SECOND operators are hard-coded, for
            // another 22 workers, since SECOND is used by GB_Matrix_wait and
            // since FIRST is useful for keeping the first tuple seen.  It is
            // controlled by the GB_INCLUDE_SECOND_OPERATOR definition, so they
            // do not appear in GB_reduce_to_* where the FIRST and SECOND
            // operators are not needed.

            // Early exit cannot be exploited, so the terminal is ignored.

            GBURBLE ("(build:assemble) ") ;
            bool done = false ;

            #ifndef GBCOMPACT

                //--------------------------------------------------------------
                // define the worker for the switch factory
                //--------------------------------------------------------------

                #define GB_INCLUDE_SECOND_OPERATOR

                #define GB_red(opname,aname) GB_red_build_ ## opname ## aname

                #define GB_RED_WORKER(opname,aname,atype)                   \
                {                                                           \
                    info = GB_red (opname, aname) ((atype *) Tx, Ti,        \
                        (atype *) S, nvals, ndupl, I_work, K_work,          \
                        tstart_slice, tnz_slice, nthreads) ;                \
                    done = (info != GrB_NO_VALUE) ;                         \
                }                                                           \
                break ;

                //--------------------------------------------------------------
                // launch the switch factory
                //--------------------------------------------------------------

                // controlled by opcode and typecode
                GB_Type_code typecode = tcode ;
                #include "GB_red_factory.c"

            #endif

            //------------------------------------------------------------------
            // generic worker
            //------------------------------------------------------------------

            if (!done)
            {
                GB_BURBLE_N (nvals, "(generic) ") ;

                //--------------------------------------------------------------
                // no typecasting, but use the fdup function pointer and memcpy
                //--------------------------------------------------------------

                // Either the fdup operator or type of S and T are
                // user-defined, or fdup is not an associative operator handled
                // by the GB_red_factory, or some combination of these
                // conditions.  User-defined types cannot be typecasted, so
                // this handles all user-defined types.

                // Tx [p] = (ttype) S [k], but with no typecasting
                #define GB_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)                \
                    memcpy (Tx +((p)*tsize), S +((k)*tsize), tsize) ;

                if (op_2nd)
                { 

                    //----------------------------------------------------------
                    // dup is the SECOND operator, with no typecasting
                    //----------------------------------------------------------

                    // Tx [p] += (ttype) S [k], but 2nd op and no typecasting
                    #define GB_ADD_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)        \
                        GB_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)
                    #include "GB_reduce_build_template.c"

                }
                else
                { 

                    //----------------------------------------------------------
                    // dup is another operator, with no typecasting needed
                    //----------------------------------------------------------

                    // Tx [p] += (ttype) S [k], but with no typecasting
                    #undef  GB_ADD_CAST_ARRAY_TO_ARRAY
                    #define GB_ADD_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)        \
                        fdup (Tx +((p)*tsize), Tx +((p)*tsize), S +((k)*tsize));
                    #include "GB_reduce_build_template.c"
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // assemble the values S into T, typecasting as needed
            //------------------------------------------------------------------

            GB_BURBLE_N (nvals, "(generic with typecast) ") ;

            // S (either S_work or S_input) must be permuted and copied into
            // T->x, since the tuples had to be sorted, or duplicates appear.
            // Any duplicates are now assembled.  Not all of the 5 types are
            // the same, but all of them are built-in since user-defined types
            // cannot be typecasted.

            GB_cast_function cast_S_to_T = GB_cast_factory (tcode, scode) ;
            GB_cast_function cast_S_to_Y = GB_cast_factory (ycode, scode) ;
            GB_cast_function cast_T_to_X = GB_cast_factory (xcode, tcode) ;
            GB_cast_function cast_Z_to_T = GB_cast_factory (tcode, zcode) ;

            ASSERT (scode <= GB_FC64_code) ;
            ASSERT (tcode <= GB_FC64_code) ;
            ASSERT (xcode <= GB_FC64_code) ;
            ASSERT (ycode <= GB_FC64_code) ;
            ASSERT (zcode <= GB_FC64_code) ;

            // Tx [p] = (ttype) S [k], with typecasting
            #undef  GB_CAST_ARRAY_TO_ARRAY
            #define GB_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)                        \
                cast_S_to_T (Tx +((p)*tsize), S +((k)*ssize), ssize) ;

            if (op_2nd)
            { 

                //--------------------------------------------------------------
                // dup operator is the SECOND operator, with typecasting
                //--------------------------------------------------------------

                // Tx [p] += (ttype) S [k], but 2nd op, with typecasting
                #undef  GB_ADD_CAST_ARRAY_TO_ARRAY
                #define GB_ADD_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)        \
                    GB_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)
                #include "GB_reduce_build_template.c"

            }
            else
            { 

                //--------------------------------------------------------------
                // dup is another operator, with typecasting required
                //--------------------------------------------------------------

                // Tx [p] += S [k], with typecasting
                #undef  GB_ADD_CAST_ARRAY_TO_ARRAY
                #define GB_ADD_CAST_ARRAY_TO_ARRAY(Tx,p,S,k)                \
                {                                                           \
                    /* ywork = (ytype) S [k] */                             \
                    GB_void ywork [GB_VLA(ysize)] ;                         \
                    cast_S_to_Y (ywork, S +((k)*ssize), ssize) ;            \
                    /* xwork = (xtype) Tx [p] */                            \
                    GB_void xwork [GB_VLA(xsize)] ;                         \
                    cast_T_to_X (xwork, Tx +((p)*tsize), tsize) ;           \
                    /* zwork = f (xwork, ywork) */                          \
                    GB_void zwork [GB_VLA(zsize)] ;                         \
                    fdup (zwork, xwork, ywork) ;                            \
                    /* Tx [tnz-1] = (ttype) zwork */                        \
                    cast_Z_to_T (Tx +((p)*tsize), zwork, zsize) ;           \
                }

                #include "GB_reduce_build_template.c"
            }
        }
    }

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    T->jumbled = false ;
    ASSERT_MATRIX_OK (T, "T built", GB0) ;
    ASSERT (GB_IS_HYPERSPARSE (T)) ;
    return (GrB_SUCCESS) ;
}

