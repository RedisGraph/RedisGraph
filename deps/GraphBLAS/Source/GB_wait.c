//------------------------------------------------------------------------------
// GB_wait:  finish all pending computations on a single matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// CALLS:     GB_builder

// This function is typically called via the GB_WAIT(A) macro, except for
// GB_assign and GB_subassign.

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

#include "GB_select.h"
#include "GB_add.h"
#include "GB_Pending.h"
#include "GB_build.h"
#include "GB_jappend.h"

#define GB_FREE_ALL                     \
{                                       \
    GB_PHIX_FREE (A) ;                  \
    GB_MATRIX_FREE (&T) ;               \
    GB_MATRIX_FREE (&S) ;               \
    GB_MATRIX_FREE (&(Aslice [0])) ;    \
    GB_MATRIX_FREE (&(Aslice [1])) ;    \
}

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
    // determine the max # of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;

    //--------------------------------------------------------------------------
    // delete zombies
    //--------------------------------------------------------------------------

    // A zombie is an entry A(i,j) in the matrix that as been marked for
    // deletion, but hasn't been deleted yet.  It is marked by "negating"
    // replacing its index i with GB_FLIP(i).  Zombies are simple to delete via
    // an in-place algorithm.  No memory is allocated so this step always
    // succeeds.  Pending tuples are ignored, so A can have pending tuples.

    GrB_Matrix T = NULL, S = NULL, Aslice [2] = { NULL, NULL } ;
    GrB_Info info = GrB_SUCCESS ;

    int64_t nzombies = A->nzombies ;

    if (nzombies > 0)
    { 
        // remove all zombies from A.  Also compute A->nvec_nonempty
        #ifdef GB_DEBUG
        int64_t anz_orig = GB_NNZ (A) ;
        #endif
        GB_OK (GB_selector (NULL, GB_NONZOMBIE_opcode, NULL, false, A,
            0, NULL, Context)) ;
        ASSERT (A->nvec_nonempty == GB_nvec_nonempty (A, NULL)) ;
        ASSERT (A->nzombies == (anz_orig - GB_NNZ (A))) ;
        A->nzombies = 0 ;
    }
    else if (A->nvec_nonempty < 0)
    { 
        // no zombies to remove, but make sure A->nvec_nonempty is computed
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;
    }

    // all the zombies are gone
    ASSERT (!GB_ZOMBIES (A)) ;

    //--------------------------------------------------------------------------
    // check for pending tuples
    //--------------------------------------------------------------------------

    int64_t anz = GB_NNZ (A) ;

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
        GB_OK (GB_ix_resize (A, anz, Context)) ;

        // conform A to its desired hypersparsity
        return (GB_to_hyper_conform (A, Context)) ;
    }

    // There are pending tuples that will now be assembled.
    ASSERT (GB_PENDING (A)) ;
    GB_Pending Pending = A->Pending ;

    //--------------------------------------------------------------------------
    // construct a new hypersparse matrix T with just the pending tuples
    //--------------------------------------------------------------------------

    // T has the same type as A->type, which can differ from the type of the
    // pending tuples, A->Pending->type.  This is OK since build process
    // assembles the tuples in the order they were inserted into the matrix.
    // The Pending->op can be NULL (an implicit SECOND function), or it
    // can be any accum operator.  The z=accum(x,y) operator can have any
    // types, and it does not have to be associative.

    info = GB_builder
    (
        &T,                     // create T
        A->type,                // T->type = A->type
        A->vlen,                // T->vlen = A->vlen
        A->vdim,                // T->vdim = A->vdim
        A->is_csc,              // T->is_csc = A->is_csc
        &(Pending->i),          // iwork_handle, becomes T->i on output
        &(Pending->j),          // jwork_handle, free on output
        &(Pending->x),          // Swork_handle, free on output
        Pending->sorted,        // tuples may or may not be sorted
        false,                  // check for duplicates
        Pending->nmax,          // size of Pending->[ijx] arrays
        true,                   // is_matrix: unused
        false,                  // ijcheck: unused
        NULL, NULL, NULL,       // original I,J,S tuples, not used here
        Pending->n,             // # of tuples
        Pending->op,            // dup operator for assembling duplicates
        Pending->type->code,    // type of Pending->x
        Context
    ) ;

    //--------------------------------------------------------------------------
    // free pending tuples
    //--------------------------------------------------------------------------

    // The tuples have been converted to T, which is more compact, and
    // duplicates have been removed.

    // This work needs to be done even if the builder fails.

    // GB_builder frees Pending->j.  If successful, Pending->i is now T->i.
    // Otherwise Pending->i is freed.  In both cases, it has been set to NULL.
    ASSERT (Pending->i == NULL) ;
    ASSERT (Pending->j == NULL) ;
    ASSERT (Pending->x == NULL) ;

    // free the list of pending tuples
    GB_Pending_free (&(A->Pending)) ;

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

    // Finally check the status of the builder.  The pending tuples, must
    // be freed (just above), whether or not the builder is successful.
    GB_OK (info) ;

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
    // determine the method for A = A+T
    //--------------------------------------------------------------------------

    // If anz > 0, T is hypersparse, even if A is a GrB_Vector
    ASSERT (T->is_hyper) ;
    ASSERT (GB_NNZ (T) > 0) ;
    ASSERT (T->nvec > 0) ;
    ASSERT (A->nvec > 0) ;

    // tjfirst = first vector in T
    int64_t tjfirst = T->h [0] ;
    int64_t anz0 = 0 ;
    int64_t kA = 0 ;
    int64_t jlast ;

    // anz0 = nnz (A0) = nnz (A (:, 0:tjfirst-1)), the region not modified by T
    if (A->is_hyper)
    { 
        // find tjfirst in A->h 
        int64_t pright = A->nvec - 1 ;
        bool found ;
        int64_t *restrict Ah = A->h ;
        GB_BINARY_SPLIT_SEARCH (tjfirst, Ah, kA, pright, found) ;
        // Ah [0 ... kA-1] excludes vector tjfirst.  The list
        // Ah [kA ... A->nvec-1] includes tjfirst.
        ASSERT (kA >= 0 && kA <= A->nvec) ;
        ASSERT (GB_IMPLIES (kA > 0 && kA < A->nvec, Ah [kA-1] < tjfirst)) ;
        ASSERT (GB_IMPLIES (found, Ah [kA] == tjfirst)) ;
        anz0 = A->p [kA] ;
        jlast = Ah [kA-1] ;
    }
    else
    { 
        kA = tjfirst ;
        anz0 = A->p [tjfirst] ;
        jlast = tjfirst - 1 ;
    }

    // anz1 = nnz (A1) = nnz (A (:, tjfirst:end)), the region modifed by T
    int64_t anz1 = anz - anz0 ;

    // A + T will have anz_new entries
    int64_t anz_new = anz + GB_NNZ (T) ;  // must have at least this space

    if (2 * anz1 < anz0)
    {

        //----------------------------------------------------------------------
        // append new tuples to A
        //----------------------------------------------------------------------

        // A is growing incrementally.  It splits into two parts: A = [A0 A1].
        // where A0 = A (:, 0:tjfirst-1) and A1 = A (:, tjfirst:end).  The
        // first part (A0 with anz = nnz (A0) entries) is not modified.  The
        // second part (A1, with anz1 = nnz (A1) entries) overlaps with T.
        // If anz1 is zero, or small compared to anz0, then it is faster to
        // leave A0 unmodified, and to update just A1.

        // FUTURE:: this does not tolerate zombies.  So do it only if A has no
        // zombies on input.  Or, when GB_add can tolerate zombies, set the
        // Aslice [1] to start at the first zombie.  Keep track of the vector
        // containing the first zombie.

        // make sure A has enough space for the new tuples
        if (anz_new > A->nzmax)
        { 
            // double the size if not enough space
            GB_OK (GB_ix_resize (A, 2*anz_new, Context)) ;
        }

        //----------------------------------------------------------------------
        // T = A1 + T
        //----------------------------------------------------------------------

        if (anz1 > 0)
        { 

            // extract A0 and A1, and then compute T = A1 + T.

            // A0 = A (:, 0:tjfirst-1), not used
            // A1 = A (:, tjfirst:end)
            // T = A1 + T

            int64_t Slice [3] ;
            Slice [0] = 0 ;         // A0 is not needed
            Slice [1] = kA ;        // kA is the first vector in A1
            Slice [2] = A->nvec ;   // A->nvec-1 is the last vector in A1
            GB_OK (GB_slice (A, 2, Slice, Aslice, Context)) ;

            ASSERT_OK (GB_check (Aslice [1], "A1 slice for GB_wait", GB0)) ;

            // free A0, which is not used
            GB_MATRIX_FREE (&(Aslice [0])) ;

            // S = A1 + T, but with no operator
            GB_OK (GB_add (&S, A->type, A->is_csc, NULL, Aslice [1], T, NULL,
                Context)) ;

            ASSERT_OK (GB_check (S, "S = A1+T", GB0)) ;

            // free A1 and T
            GB_MATRIX_FREE (&T) ;
            GB_MATRIX_FREE (&(Aslice [1])) ;

            // replace T with S
            T = S ;
            S = NULL ;

            // remove A1 from the vectors of A
            if (A->is_hyper)
            { 
                A->nvec = kA ;
            }
        }

        //----------------------------------------------------------------------
        // append T to the end of A0
        //----------------------------------------------------------------------

        int64_t *restrict Ai = A->i ;
        GB_void *restrict Ax = A->x ;
        int64_t asize = A->type->size ;

        const int64_t *restrict Tp = T->p ;
        const int64_t *restrict Th = T->h ;
        const int64_t *restrict Ti = T->i ;
        const GB_void *restrict Tx = T->x ;
        int64_t tnvec = T->nvec ;
        int64_t tnz = GB_NNZ (T) ;

        anz = anz0 ;
        int64_t anz_last = anz ;
    
        int nthreads = GB_nthreads (tnz, chunk, nthreads_max) ;

        // append the indices and values of T to the end of A
        GB_memcpy (Ai + anz        , Ti, tnz * sizeof (int64_t), nthreads) ;
        GB_memcpy (Ax + anz * asize, Tx, tnz * asize           , nthreads) ;

        // append the vectors of T to the end of A
        for (int64_t k = 0 ; k < tnvec ; k++)
        { 
            int64_t j = Th [k] ;
            ASSERT (j >= tjfirst) ;
            anz += (Tp [k+1] - Tp [k]) ;
            GB_OK (GB_jappend (A, j, &jlast, anz, &anz_last, Context)) ;
        }

        GB_jwrapup (A, jlast, anz) ;
        ASSERT (anz == anz_new) ;

        // recompute the # of non-empty vectors
        A->nvec_nonempty = GB_nvec_nonempty (A, Context) ;

        ASSERT_OK (GB_check (A, "A after GB_wait:append", GB0)) ;

        GB_MATRIX_FREE (&T) ;

        // conform A to its desired hypersparsity
        return (GB_to_hyper_conform (A, Context)) ;

    }
    else
    { 

        //----------------------------------------------------------------------
        // A = A+T
        //----------------------------------------------------------------------

        // The update is not incremental since most of A is changing.  Just do
        // a single parallel add: S=A+T, free T, and then transplant S back
        // into A.  The nzmax of A is tight, with no room for future
        // incremental growth.

        // FUTURE:: if GB_add could tolerate zombies in A, then the initial
        // prune of zombies can be skipped.

        GB_OK (GB_add (&S, A->type, A->is_csc, NULL, A, T, NULL, Context)) ;
        GB_MATRIX_FREE (&T) ;
        ASSERT_OK (GB_check (S, "S after GB_wait:add", GB0)) ;
        return (GB_transplant_conform (A, A->type, &S, Context)) ;
    }
}

