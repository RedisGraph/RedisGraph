//------------------------------------------------------------------------------
// GB_wait:  finish all pending computations on a single matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The matrix A has zombies and/or pending tuples placed there by
// GrB_setElement and GrB_*assign.  Zombies must now be deleted, and pending
// tuples must now be assembled together and added into the matrix.

// Only GrB_SUCCESS and GrB_OUT_OF_MEMORY are returned by this function.

// When the function returns, the matrix has been removed from the queue
// and all pending tuples and zombies have been deleted.  This is true even
// the function fails due to lack of memory (in that case, the matrix is
// cleared as well).

// GrB_wait removes the head of the queue from the queue via
// GB_queue_remove_head, and then passes the matrix to this function.  Thus is
// is possible (and safe) for this matrix to operate on a matrix not in
// the queue.

#include "GB.h"

GrB_Info GB_wait                // finish all pending computations
(
    GrB_Matrix A                // matrix with pending computations
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // The matrix A might have pending operations but not be in the queue.
    // GB_check expects the matrix to be in the queue.
    // ASSERT_OK (GB_check (A, "A to wait", 0)) ;

    int64_t ncols = A->ncols ;
    int64_t *Ap = A->p ;
    int64_t *Ai = A->i ;
    void    *Ax = A->x ;
    int64_t s = A->type->size ;

    //--------------------------------------------------------------------------
    // delete zombies
    //--------------------------------------------------------------------------

    // A zombie is an entry A(i,j) in the matrix that as been marked for
    // deletion, but hasn't been deleted yet.  It is marked by "negating"
    // replacing its row index i with FLIP(i).  Zombies are simple to delete
    // via an in-place algorithm.  No memory is allocated so this step always
    // succeeds.  Pending tuples are ignored, so A can have pending tuples.

    if (A->nzombies > 0)
    {

        // There are zombies that will now be deleted.
        ASSERT (ZOMBIES_OK (A)) ;
        ASSERT (ZOMBIES (A)) ;

        // This step tolerates pending tuples
        // since pending tuples and zombies do not intersect
        ASSERT (PENDING_OK (A)) ;

        //----------------------------------------------------------------------
        // zombies exist in the matrix: delete them all
        //----------------------------------------------------------------------

        int64_t anz = 0 ;

        for (int64_t j = 0 ; j < ncols ; j++)
        {
            // get the start of the current column j
            int64_t p = Ap [j] ;

            // log the start of the new column j
            Ap [j] = anz ;

            ASSERT (anz <= p) ;

            for ( ; p < Ap [j+1] ; p++)
            {
                int64_t i = Ai [p] ;
                if (IS_NOT_ZOMBIE (i))
                {
                    // A(i,j) is not a zombie, keep it
                    Ai [anz] = i ;
                    // Ax [anz] = Ax [p] ;
                    if (anz != p)
                    {
                        memcpy (Ax +(anz*s), Ax +(p*s), s) ;
                    }
                    anz++ ;
                }
            }
        }

        // log the end of the last column
        int64_t anz_old = Ap [ncols] ;
        Ap [ncols] = anz ;

        // exactly A->nzombies have been deleted from A
        ASSERT (A->nzombies == (anz_old - anz)) ;

        // at least one zombie has been deleted
        ASSERT (anz < anz_old) ;

        // no more zombies; pending tuples may still exist
        A->nzombies = 0 ;
        ASSERT (A->npending >= 0) ;
    }

    //--------------------------------------------------------------------------
    // check for pending tuples
    //--------------------------------------------------------------------------

    // all the zombies are gone
    ASSERT (!ZOMBIES (A)) ;

    if (A->npending == 0)
    {
        // nothing more to do; remove the matrix from the queue
        ASSERT (!PENDING (A)) ;
        GB_queue_remove (A) ;
        ASSERT (!(A->enqueued)) ;
        // trim the extra space from the matrix
        bool ok = GB_Matrix_realloc (A, NNZ (A), true, NULL) ;
        ASSERT (ok) ;
        ASSERT_OK (GB_check (A, "A after just killing zombies", 0)) ;
        return (REPORT_SUCCESS) ;
    }

    // There are pending tuples that will now be assembled.
    ASSERT (PENDING (A)) ;

    //--------------------------------------------------------------------------
    // construct a new matrix with just the pending tuples
    //--------------------------------------------------------------------------

    // [ be careful; T->magic is zero because T->p is malloc'd not calloc'd
    GrB_Info info ;
    GrB_Matrix T ;
    GB_NEW (&T, A->type, A->nrows, ncols, false, true) ;
    if (info != GrB_SUCCESS)
    {
        // out of memory; clear all of A and remove from queue
        GB_Matrix_clear (A) ;
        ASSERT (!(A->enqueued)) ;
        ASSERT (!PENDING (A)) ;
        ASSERT (info == GrB_OUT_OF_MEMORY) ;
        ASSERT_OK (GB_check (A, "A had to cleared (out of memory)", 0)) ;
        return (info) ;
    }

    // No typecasting is needed.
    ASSERT (T->type == A->type) ;

    // if NULL operator: an implicit 'SECOND' function will be used
    // otherwise use A->operator_pending
    info = GB_builder (T, &(A->ipending), &(A->jpending),
        A->sorted_pending, A->xpending, A->npending, A->max_npending,
        A->operator_pending,  A->type->code) ;

    //--------------------------------------------------------------------------
    // free pending tuples
    //--------------------------------------------------------------------------

    // The tuples have been converted to T, which is more compact (except
    // for T->p of size ncols+1) and duplicates have been removed.
    ASSERT (NNZ (T) <= A->npending) ;

    // This work needs to be done even if the builder fails.

    // GB_builder frees A->jpending.  If successful, A->ipending is now T->i.
    // Otherwise A->ipending is freed.  In both cases, it has been set to NULL.
    ASSERT (A->ipending == NULL && A->jpending == NULL) ;

    // pending tuples are now free; so A->xpending can be freed as well
    GB_free_pending (A) ;

    //--------------------------------------------------------------------------
    // remove the matrix from the queue
    //--------------------------------------------------------------------------

    ASSERT (!PENDING (A)) ;
    ASSERT (!ZOMBIES (A)) ;
    GB_queue_remove (A) ;

    // No pending operations on A, and A is not in the queue, so GB_check can
    // now see the conditions it expects.
    ASSERT (!(A->enqueued)) ;
    ASSERT_OK (GB_check (A, "A after moving pending tuples to T", 0)) ;

    //--------------------------------------------------------------------------
    // check the status of the builder
    //--------------------------------------------------------------------------

    // Finally check the status of the builder.  The pending tuples, just freed
    // above, must be freed whether or not the builder is succesful.
    if (info != GrB_SUCCESS)
    {
        // out of memory; clear all of A
        GB_MATRIX_FREE (&T) ;
        GB_Matrix_clear (A) ;
        ASSERT (info == GrB_OUT_OF_MEMORY) ;
        ASSERT_OK (GB_check (A, "A had to cleared (out of memory)", 0)) ;
        return (info) ;
    }

    // T is now initialized.  T itself has no list of pending tuples. ]
    ASSERT (T->magic == MAGIC) ;
    ASSERT_OK (GB_check (T, "T = matrix of pending tuples", 0)) ;
    ASSERT (!PENDING (T)) ;
    ASSERT (!ZOMBIES (T)) ;

    //--------------------------------------------------------------------------
    // check for quick transplant
    //--------------------------------------------------------------------------

    if (NNZ (A) == 0)
    {
        // A has no entries so just transplant T into A.  This takes no work at
        // all since A and T have the same type, and since T is not shallow.
        // GB_Matrix_transplant can return GrB_SUCCESS or GrB_OUT_OF_MEMORY, in
        // general, but in this case it cannot fail since no memory is
        // allocated.  A and T have the same type, and T is not shallow, so no
        // memory is allocated.  The return value is thus always GrB_SUCCESS.
        ASSERT (A->type == T->type) ;
        ASSERT (A->p != NULL && !A->p_shallow) ;
        ASSERT (T->p != NULL && !T->p_shallow) ;
        return (GB_Matrix_transplant (A, A->type, &T)) ;
        // T has been freed and its content is now in A
    }

    //--------------------------------------------------------------------------
    // reallocate A to hold the tuples, growing it in size
    //--------------------------------------------------------------------------

    // add space to A to accomodate the new tuples
    double memory = 0 ;
    if (!GB_Matrix_realloc (A, NNZ (A) + NNZ (T), true, &memory))
    {
        // out of memory; clear all of A
        GB_MATRIX_FREE (&T) ;
        GB_Matrix_clear (A) ;
        ASSERT_OK (GB_check (A, "A had to cleared (out of memory)", 0)) ;
        return (ERROR (GrB_OUT_OF_MEMORY, (LOG,
            "Out of memory to assemble matrix; %g GB required", memory)));
    }

    // reacquire the pointers since they have likely moved
    Ai = A->i ;
    Ax = A->x ;

    ASSERT_OK (GB_check (A, "A increased in size", 0)) ;

    //--------------------------------------------------------------------------
    // A = A + T ; in place by folding in the tuples in reverse order
    //--------------------------------------------------------------------------

    // No addition is done since the nonzero patterns of A and T are disjoint.

    // Debug code is left in place, commented out, because this in-place
    // reverse order folding operation is rather delicate.  The debug code was
    // critical in writing the code, and it also helps to explain what the code
    // does.  In particular, see the 'Apcopy' array which is a copy of the
    // original values in A->p.  The A->p array is safely overwritten but the
    // debug assertion "pa == Apcopy [j+1]-1" cannot be done on the modified
    // A->p.  It must be done on the original A->p.

    int64_t *Tp = T->p ;
    int64_t *Ti = T->i ;
    void    *Tx = T->x ;

    // pdest points to the top of the stack at the end of the A matrix;
    // this is also the total number of nonzeros in A and T
    int64_t pdest = Ap [ncols] + Tp [ncols] ;

    // pa points to the last entry at the end of the current A matrix
    int64_t pa = Ap [ncols] - 1 ;

    // pt points to the last entry of the T matrix
    int64_t pt = Tp [ncols] - 1 ;

    // copy Ap into Apcopy, for testing
    // int64_t *Apcopy = malloc (ncols+1, sizeof (int64_t)) ;
    // for (int64_t j = 0 ; j <= ncols ; j++) Apcopy [j] = Ap [j] ;

    // log the new end of the last column
    Ap [ncols] = pdest ;

    // merge in the tuples into each column
    for (int64_t j = ncols-1 ; j >= 0 ; j--)
    {

        //----------------------------------------------------------------------
        // merge A (:,j) and T (:,j) in reverse order
        //----------------------------------------------------------------------

        // A (:,j) is in Ai,Ax [pa_end+1 ... pa]
        int64_t pa_end = Ap [j] - 1 ;
        //ASSERT (pa == Apcopy [j+1] - 1) ;

        // T (:,j) is in Ti,Tx [pt_end+1 ... pt]
        int64_t pt_end = Tp [j] - 1 ;
        ASSERT (pt == Tp [j+1] - 1) ;

        //----------------------------------------------------------------------
        // merge while entries exist in both A (:,j) and T (:,j)
        //----------------------------------------------------------------------

        while (pa > pa_end && pt > pt_end)
        {
            // entries exist in both A (:,j) and T (:,j); take the biggest one
            int64_t ia = Ai [pa] ;
            int64_t it = Ti [pt] ;

            // no entries are both in A and T
            ASSERT (ia != it) ;

            // prepare the destination
            --pdest ;
            ASSERT (pa < pdest) ;

            if (ia > it)
            {
                // move Ai,Ax [pa] onto the stack
                Ai [pdest] = ia ;
                // Ax [pdest] = Ax [pa]
                memcpy (Ax +(pdest*s), Ax +(pa*s), s) ;
                --pa ;
            }
            else // it > ia
            {
                // move Ti,Tx [pt] onto the stack
                Ai [pdest] = it ;
                // Ax [pdest] = Tx [pt]
                memcpy (Ax +(pdest*s), Tx +(pt*s), s) ;
                --pt ;
            }
        }

        //----------------------------------------------------------------------
        // merge the remainder
        //----------------------------------------------------------------------

        // Either A (:,j) or T (:,j) is exhausted; but the other one can have
        // entries that still need to be shifted down.

        while (pa > pa_end)
        {
            // entries still exist in A (:,j); shift downwards
            int64_t ia = Ai [pa] ;

            // prepare the destination
            --pdest ;
            ASSERT (pa <= pdest) ;

            // move Ai,Ax [pa] onto the stack
            Ai [pdest] = ia ;
            // Ax [pdest] = Ax [pa]
            if (pa != pdest)
            {
                memcpy (Ax +(pdest*s), Ax +(pa*s), s) ;
            }
            --pa ;
        }

        while (pt > pt_end)
        {
            // entries still exist in T (:,j); shift downwards
            int64_t it = Ti [pt] ;

            // prepare the destination
            --pdest ;

            // move Ai,Ax [pa] onto the stack
            Ai [pdest] = it ;
            // Ax [pdest] = Ax [pa]
            memcpy (Ax +(pdest*s), Tx +(pt*s), s) ;
            --pt ;
        }

        //----------------------------------------------------------------------
        // column j is done; record its new position
        //----------------------------------------------------------------------

        ASSERT (Ap [j] <= pdest) ;
        if (Ap [j] == pdest)
        {
            // no tuples left
            ASSERT (pt == -1) ;
            break ;
        }

        Ap [j] = pdest ;
    }

    // uncomment this for Apcopy debugging
    // free (Apcopy) ;

    //--------------------------------------------------------------------------
    // tuples have now been assembled into the matrix
    //--------------------------------------------------------------------------

    GB_MATRIX_FREE (&T) ;
    ASSERT_OK (GB_check (A, "A after assembling pending tuples", 0)) ;
    return (REPORT_SUCCESS) ;
}

