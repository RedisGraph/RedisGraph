//------------------------------------------------------------------------------
// GB_wait:  finish all pending computations on a single matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The matrix A has zombies and/or pending tuples placed there by
// GrB_setElement and GrB_*assign.  Zombies must now be deleted, and pending
// tuples must now be assembled together and added into the matrix.

// When the function returns, the matrix has been removed from the queue
// and all pending tuples and zombies have been deleted.  This is true even
// the function fails due to lack of memory (in that case, the matrix is
// cleared as well).

// GrB_wait removes the head of the queue from the queue via
// GB_queue_remove_head, and then passes the matrix to this function.  Thus is
// is possible (and safe) for this matrix to operate on a matrix not in
// the queue.

// If A is hypersparse, the time taken is at most O(nnz(A) + t log t), where t
// is the number of pending tuples in A, and nnz(A) includes both zombies and
// live entries.  There is no O(m) or O(n) time component, if A is m-by-n.
// If the number of non-empty vectors of A grows too large, then A can be
// converted to non-hypersparse.

// If A is non-hypersparse, then O(n) is added in the worst case, to prune
// zombies and to update the vector pointers for A.

#include "GB.h"

GrB_Info GB_wait                // finish all pending computations
(
    GrB_Matrix A,               // matrix with pending computations
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (A != NULL) ;

    // The matrix A might have pending operations but not be in the queue.
    // GB_check expects the matrix to be in the queue.  As a result, GB_check
    // can report an inconsistency, and thus this assert must be made
    // with a negative pr.
    ASSERT_OK (GB_check (A, "A to wait", GB_FLIP (GB0))) ;

    //--------------------------------------------------------------------------
    // delete zombies
    //--------------------------------------------------------------------------

    // A zombie is an entry A(i,j) in the matrix that as been marked for
    // deletion, but hasn't been deleted yet.  It is marked by "negating"
    // replacing its index i with GB_FLIP(i).  Zombies are simple to delete via
    // an in-place algorithm.  No memory is allocated so this step always
    // succeeds.  Pending tuples are ignored, so A can have pending tuples.

    GrB_Info info = GrB_SUCCESS ;
    int64_t anz = GB_NNZ (A) ;
    int64_t anz_orig = anz ;
    int64_t anzmax_orig = A->nzmax ;
    ASSERT (anz_orig <= anzmax_orig) ;

    int64_t nzombies = A->nzombies ;

    if (nzombies > 0)
    { 

        // There are zombies that will now be deleted.
        ASSERT (GB_ZOMBIES_OK (A)) ;
        ASSERT (GB_ZOMBIES (A)) ;

        // This step tolerates pending tuples
        // since pending tuples and zombies do not intersect
        ASSERT (GB_PENDING_OK (A)) ;

        //----------------------------------------------------------------------
        // zombies exist in the matrix: delete them all
        //----------------------------------------------------------------------

        // compare with the pruning phase of GB_resize
        #define GB_PRUNE if (GB_IS_ZOMBIE (i)) continue ;
        #include "GB_prune_inplace.c"

        //----------------------------------------------------------------------
        // all zombies have been deleted
        //----------------------------------------------------------------------

        // exactly A->nzombies have been deleted from A
        ASSERT (A->nzombies == (anz_orig - anz)) ;

        // at least one zombie has been deleted
        ASSERT (anz < anz_orig) ;

        // no more zombies; pending tuples may still exist
        A->nzombies = 0 ;
        ASSERT (GB_PENDING_OK (A)) ;

        // A->nvec_nonempty has been updated
        ASSERT (A->nvec_nonempty == GB_nvec_nonempty (A)) ;
    }

    ASSERT (anz == GB_NNZ (A)) ;

    //--------------------------------------------------------------------------
    // check for pending tuples
    //--------------------------------------------------------------------------

    // all the zombies are gone
    ASSERT (!GB_ZOMBIES (A)) ;

    if (!GB_PENDING (A))
    { 
        // nothing more to do; remove the matrix from the queue
        ASSERT (!GB_PENDING (A)) ;
        GB_CRITICAL (GB_queue_remove (A)) ;
        ASSERT (!(A->enqueued)) ;

        // trim any significant extra space from the matrix, but allow for some
        // future insertions.  do not increase the size of the matrix;
        // zombies have been deleted but no pending tuples added.  This is
        // guaranteed not to fail.
        ASSERT (anz <= anz_orig) ;
        info = GB_ix_resize (A, anz, Context) ;
        ASSERT (info == GrB_SUCCESS) ;

        // conform A to its desired hypersparsity
        return (GB_to_hyper_conform (A, Context)) ;
    }

    // There are pending tuples that will now be assembled.
    ASSERT (GB_PENDING (A)) ;

    //--------------------------------------------------------------------------
    // construct a new hypersparse matrix T with just the pending tuples
    //--------------------------------------------------------------------------

    // If anz > 0, T is always hypersparse.  Otherwise T can be returned as
    // non-hypersparse, and it is then transplanted as-is into the final A.

    // T has the same type as A->type, which can differ from the type of the
    // pending tuples, A->type_pending.  This is OK since build process
    // assembles the tuples in the order they were inserted into the matrix.
    // The A->operator_pending can be NULL (an implicit SECOND function), or it
    // can be any accum operator.  The z=accum(x,y) operator can have any
    // types, and it does not have to be associative.

    GrB_Matrix T ;
    info = GB_builder (&T, A->type, A->vlen, A->vdim, A->is_csc,
        &(A->i_pending), &(A->j_pending), A->sorted_pending, A->s_pending,
        A->n_pending, A->max_n_pending, A->operator_pending,
        A->type_pending->code, Context) ;

    //--------------------------------------------------------------------------
    // free pending tuples
    //--------------------------------------------------------------------------

    // The tuples have been converted to T, which is more compact, and
    // duplicates have been removed.

    // This work needs to be done even if the builder fails.

    // GB_builder frees A->j_pending.  If successful, A->i_pending is now T->i.
    // Otherwise A->i_pending is freed.  In both cases, it has been set to NULL.
    ASSERT (A->i_pending == NULL && A->j_pending == NULL) ;

    // pending tuples are now free; so A->s_pending can be freed as well
    // FUTURE: GB_builder could modify A->s_pending in place to save memory,
    // but it can't do that for the user's S array for GrB_*_build.
    GB_pending_free (A) ;

    //--------------------------------------------------------------------------
    // remove the matrix from the queue
    //--------------------------------------------------------------------------

    ASSERT (!GB_PENDING (A)) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    GB_CRITICAL (GB_queue_remove (A)) ;

    // No pending operations on A, and A is not in the queue, so GB_check can
    // now see the conditions it expects.
    ASSERT (!(A->enqueued)) ;
    ASSERT_OK (GB_check (A, "A after moving pending tuples to T", GB0)) ;

    //--------------------------------------------------------------------------
    // check the status of the builder
    //--------------------------------------------------------------------------

    // Finally check the status of the builder.  The pending tuples, just freed
    // above, must be freed whether or not the builder is succesful.
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_CONTENT_FREE (A) ;
        ASSERT (T == NULL) ;
        return (info) ;
    }

    ASSERT_OK (GB_check (T, "T = matrix of pending tuples", GB0)) ;
    ASSERT (!GB_PENDING (T)) ;
    ASSERT (!GB_ZOMBIES (T)) ;
    ASSERT (GB_NNZ (T) > 0) ;
    ASSERT (T->is_hyper) ;
    ASSERT (T->nvec == T->nvec_nonempty) ;

    //--------------------------------------------------------------------------
    // check for quick transplant
    //--------------------------------------------------------------------------

    if (anz == 0)
    { 
        // A has no entries so just transplant T into A, then free T and
        // conform A to its desired hypersparsity.
        return (GB_transplant_conform (A, A->type, &T, Context)) ;
    }

    //--------------------------------------------------------------------------
    // reallocate A to hold the tuples
    //--------------------------------------------------------------------------

    // make A->nzmax larger to accomodate future tuples, but only
    // allocate new space if the old A->nzmax is insufficient.

    int64_t anz_new = anz + GB_NNZ (T) ;  // must have at least this space

    info = GB_ix_resize (A, anz_new, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_MATRIX_FREE (&T) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // if A is hypersparse, ensure A->plen is sufficient for A=A+T
    //--------------------------------------------------------------------------

    // If anz > 0, T is hypersparse, even if A is a GrB_Vector
    ASSERT (T->is_hyper) ;

    // No addition is done since the nonzero patterns of A and T are disjoint.

    int64_t *restrict Ah = A->h ;
    int64_t *restrict Ap = A->p ;
    int64_t *restrict Ai = A->i ;
    GB_void *restrict Ax = A->x ;
    int64_t anvec = A->nvec ;
    int64_t anvec_new = anvec ;

    const int64_t *restrict Th = T->h ;
    const int64_t *restrict Tp = T->p ;
    const int64_t *restrict Ti = T->i ;
    const GB_void *restrict Tx = T->x ;
    int64_t tnvec = T->nvec ;

    int64_t ak, tk ;

    if (A->is_hyper)
    {

        // 2-way merge of A->h and T->h
        for (ak = 0, tk = 0 ; ak < anvec && tk < tnvec ; )
        {
            int64_t ja = Ah [ak] ;
            int64_t jt = Th [tk] ;
            if (jt == ja)
            { 
                // vector jt appears in both A and T
                ak++ ;
                tk++ ;
            }
            else if (ja < jt)
            { 
                // vector ja appears in A but not T
                ak++ ;
            }
            else // jt < ja
            { 
                // vector jt appears in T but not A
                tk++ ;
                anvec_new++ ;
            }
        }

        // count the vectors not yet seen in T
        if (tk < tnvec)
        { 
            anvec_new += (tnvec - tk) ;
        }

        // reallocate A->p and A->h, if needed
        if (anvec_new > A->plen)
        {
            if (GB_to_nonhyper_test (A, anvec_new, A->vdim))
            { 
                // convert to non-hypersparse if anvec_new will become too large
                info = GB_to_nonhyper (A, Context) ;
            }
            else
            { 
                // increase the size of A->p and A->h.  The size must be at
                // least anvec_new, but add some slack for future growth.
                int64_t aplen_new = 2 * (anvec_new + 1) ;
                aplen_new = GB_IMIN (aplen_new, A->vdim) ;
                info = GB_hyper_realloc (A, aplen_new, Context) ;
            }
            if (info != GrB_SUCCESS)
            { 
                // out of memory; all content of A has been freed
                ASSERT (A->magic == GB_MAGIC2) ;
                GB_MATRIX_FREE (&T) ;
                return (info) ;
            }
            Ah = A->h ;
            Ap = A->p ;
        }
    }

    ASSERT_OK (GB_check (A, "A after increasing A->h", GB0)) ;
    ASSERT_OK (GB_check (T, "T to fold in", GB0)) ;

    //--------------------------------------------------------------------------
    // A = A + T ; in place by folding in the tuples in reverse order
    //--------------------------------------------------------------------------

    // Merge in the tuples into each vector, in reverse order.  Note that Ap
    // [k+1] or Ap [j+1] is changed during the iteration.  The bottom of the
    // new A is treated like a stack, where entries are placed on top of the Ai
    // and Ax stack, and vector indices are placed on top of the Ah stack
    // if A is hypersparse.

    // T is always hypersparse, even if A and T are typecasted GrB_Vector
    // objects.  A can be non-hypersparse or hypersparse.  If A is hypersparse
    // then this step does not take O(A->vdim) time.  It takes at most
    // O(nnz(Z)+nnz(A)) time, regardless of the vector dimension of A and T,
    // A->vdim and T->vdim.

    bool A_is_hyper = A->is_hyper ;

    int64_t asize = A->type->size ;

    // pdest points to the top of the stack at the end of the A matrix;
    // this is also the total number of nonzeros in A+T.  Since the stack
    // is empty, pdest points to one past the position where the last entry
    // in A will appear.
    int64_t pdest = anz_new ;

    // pdest-1 must be within the size of A->i and A->x
    ASSERT (pdest <= A->nzmax) ;

    tk = tnvec - 1 ;

    // ak_dest points to the top of the hyperlist stack, also currently empty.
    int64_t ak_dest ;
    if (A_is_hyper)
    { 
        // Ah [ak] is the rightmost non-empty vector in the hypersparse A.
        // It will be moved to Ah [anvec_new-1].
        ak = A->nvec - 1 ;
        ak_dest = anvec_new ;
    }
    else
    { 
        // ak is the rightmost vector in the non-hypersparse A
        ak = A->vdim - 1 ;
        ak_dest = A->vdim ;
        ASSERT (A->nvec == A->vdim) ;
    }

    // count the number of non-empty vectors (again, if hypersparse, but for
    // the first time if non-hypersparse)
    anvec_new = A->nvec_nonempty ;

    // while T has non-empty vectors
    while (tk >= 0)
    {

        // When T is exhausted, the while loop can stop.  Let j1 be the
        // leftmost non-empty vector of the hypersparse T.  A(:,0:j1-1) is
        // not affected by the merge.  Only vectors A(:,j1:n-1) need to be
        // shifted (where n == A->vdim).

        // If the vectors of A are exhausted, ak becomes -1 and stays there.
        ASSERT (ak >= -1) ;

        //----------------------------------------------------------------------
        // get vectors A(:,j) and T(:,j)
        //----------------------------------------------------------------------

        int64_t j, ja, jt, pa, pa_end, pt, pt_end ;
        if (A_is_hyper)
        { 
            // get the next non-empty vector ja in the prior hypersparse A
            ja = (ak >= 0) ? Ah [ak] : -1 ;
        }
        else
        { 
            // ja always appears in the non-hypersparse A
            ja = ak ;
        }

        // jt is the next non-empty vector in the hypersparse T
        jt = Th [tk] ;

        ASSERT (jt >= 0) ;
        ASSERT (ja >= -1) ;

        if (ja == jt)
        { 
            // vector j appears in both A(:,j) and T(:,j)
            ASSERT (ak >= 0) ;
            j = ja ;
            pa_end = Ap [ak  ] - 1 ;
            pa     = Ap [ak+1] - 1 ;
            pt_end = Tp [tk  ] - 1 ;
            pt     = Tp [tk+1] - 1 ;
        }
        else if (ja > jt)
        { 
            // vector j appears in A(:,j) but not T(:,j)
            ASSERT (ak >= 0) ;
            j = ja ;
            pa_end = Ap [ak  ] - 1 ;
            pa     = Ap [ak+1] - 1 ;
            pt_end = -1 ;
            pt     = -1 ;
        }
        else // jt > ja
        { 
            // vector j appears in T(:,j) but not A(:,j)
            ASSERT (ak >= -1) ;
            j = jt ;
            pa_end = -1 ;
            pa     = -1 ;
            pt_end = Tp [tk  ] - 1 ;
            pt     = Tp [tk+1] - 1 ;
        }

        ASSERT (j >= 0 && j < A->vdim) ;

        // A (:,j) is in Ai,Ax [pa_end+1 ... pa]
        // T (:,j) is in Ti,Tx [pt_end+1 ... pt]

        //----------------------------------------------------------------------
        // count the number of non-empty vectors in the new A
        //----------------------------------------------------------------------

        if (!(pa > pa_end) && (pt > pt_end))
        { 
            // A(:,j) is empty but T(:,j) is not; count one more non-empty
            // vector in A
            anvec_new++ ;
        }

        //----------------------------------------------------------------------
        // log the new end of A(:,j)
        //----------------------------------------------------------------------

        // get the next free slot on the hyperlist stack
        ASSERT (ak < ak_dest) ;
        ASSERT (GB_IMPLIES (!A_is_hyper, ak_dest == ak+1)) ;
        --ak_dest ;
        ASSERT (ak <= ak_dest) ;
        ASSERT (ak_dest >= 0) ;

        if (A_is_hyper)
        { 
            // push j onto the stack for the new hyperlist for A
            Ah [ak_dest] = j ;
        }

        ASSERT (GB_IMPLIES (!A_is_hyper, ak_dest == ak && j == ak)) ;
        Ap [ak_dest+1] = pdest ;

        //----------------------------------------------------------------------
        // merge while entries exist in both A (:,j) and T (:,j) (reverse order)
        //----------------------------------------------------------------------

        while (pa > pa_end && pt > pt_end)
        {
            // entries exist in both A (:,j) and T (:,j); take the biggest one
            int64_t ia = Ai [pa] ;
            int64_t it = Ti [pt] ;

            // no entries are both in A and T
            ASSERT (ia != it) ;

            // get next free slot on the top of the stack of the entries of A
            --pdest ;
            ASSERT (pa < pdest) ;

            if (ia > it)
            { 
                // push Ai,Ax [pa] onto the stack
                Ai [pdest] = ia ;
                // Ax [pdest] = Ax [pa]
                memcpy (Ax +(pdest*asize), Ax +(pa*asize), asize) ;
                --pa ;
            }
            else // it > ia
            { 
                // push Ti,Tx [pt] onto the stack
                ASSERT (it > ia) ;
                Ai [pdest] = it ;
                // Ax [pdest] = Tx [pt]
                memcpy (Ax +(pdest*asize), Tx +(pt*asize), asize) ;
                --pt ;
            }
        }

        //----------------------------------------------------------------------
        // merge the remainder
        //----------------------------------------------------------------------

        // Either A (:,j) or T (:,j) is exhausted; but the other one can have
        // entries that still need to be shifted down.

        // FUTURE: can use two memmove's here, for Ai and Ax, with no while
        // loop, since the source and destination can overlap
        while (pa > pa_end)
        {
            // entries still exist in A (:,j); shift downwards
            int64_t ia = Ai [pa] ;

            // get next free slot on the top of the stack of the entries of A
            --pdest ;
            ASSERT (pa <= pdest) ;

            // push Ai,Ax [pa] onto the stack
            if (pa != pdest)
            { 
                Ai [pdest] = ia ;
                // Ax [pdest] = Ax [pa]
                memcpy (Ax +(pdest*asize), Ax +(pa*asize), asize) ;
            }
            --pa ;
        }

        // FUTURE: can use two memcpy's here, for Ai and Ax, with no while loop
        while (pt > pt_end)
        { 
            // entries still exist in T (:,j); shift downwards
            int64_t it = Ti [pt] ;

            // get next free slot on the top of the stack of the entries of A
            --pdest ;

            // push Ti,Tx [pt] onto the stack
            Ai [pdest] = it ;
            // Ax [pdest] = Tx [pt]
            memcpy (Ax +(pdest*asize), Tx +(pt*asize), asize) ;
            --pt ;
        }

        //----------------------------------------------------------------------
        // advance to the next vector (right to left)
        //----------------------------------------------------------------------

        if (ja == jt)
        { 
            // vector j appears in both A(:,j) and T(:,j)
            --ak ;
            --tk ;
        }
        else if (ja > jt)
        { 
            // vector j appears in A(:,j) but not T(:,j)
            --ak ;
        }
        else // jt > ja
        { 
            // vector j appears in T(:,j) but not A(:,j)
            --tk ;
        }
    }

    // update the count of non-empty vectors in A
    A->nvec_nonempty = anvec_new ;

    // all vectors have been merged into A
    if (A->is_hyper)
    { 
        A->nvec = A->nvec_nonempty ;
    }

    // end condition: no need to log the end of A(:,-1) since Ap[0]=0
    // already holds.
    ASSERT (Ap [0] == 0) ;

    //--------------------------------------------------------------------------
    // tuples have now been assembled into the matrix
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&T) ;
    ASSERT_OK (GB_check (A, "A after assembling pending tuples", GB0)) ;

    // conform A to its desired hypersparsity
    return (GB_to_hyper_conform (A, Context)) ;
}

