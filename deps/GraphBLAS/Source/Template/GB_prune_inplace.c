//------------------------------------------------------------------------------
// GB_prune_inplace: prune a matrix in place
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// The function that #include's this file defines a GB_PRUNE macro that defines
// how entries are pruned in place from the matrix.  This task is similar to
// GB_select, except that here the pruning is done in place, whereas GB_select
// constructs a new matrix T with the pruned version of A.

// This code is used by GB_wait to delete zombies, and by GB_resize to delete
// entries outside the resized dimensions, if A->vlen decreases.

{

    // GB_for_each_* cannot be used since A->p is changing
    int64_t *restrict Ah = A->h ;
    int64_t *restrict Ap = A->p ;
    int64_t *restrict Ai = A->i ;
    GB_void *restrict Ax = A->x ;

    int64_t p = 0 ;
    int64_t vdim = A->vdim ;

    // like as GB_jstartup, except A->nvec is not cleared
    anz = 0 ;
    int64_t anz_last = 0 ;
    int64_t asize = A->type->size ;
    ASSERT (Ap [0] == 0) ;

    int64_t anvec_new = 0 ;

    if (A->is_hyper)
    {

        //----------------------------------------------------------------------
        // prune entries from a hypersparse matrix
        //----------------------------------------------------------------------

        // vectors are deleted if they become empty
        ASSERT (A->nvec <= A->plen) ;
        int64_t nvec = A->nvec ;    // current nvec
        A->nvec = 0 ;               // nvec after pruning empty vectors

        // GB_for_each_vector (A): but where A->p and A->h are changing:
        for (int64_t k = 0 ; k < nvec ; k++)
        {
            // GB_for_each_entry (j, p, pend)) but A->p is changing:
            int64_t j = Ah [k] ;
            int64_t pend = Ap [k+1] ;
            for ( ; p < pend ; p++)
            {
                int64_t i = Ai [p] ;
                GB_PRUNE ;                     // test, then break or continue
                // keep this entry
                if (anz != p)
                { 
                    Ai [anz] = i ;
                    memcpy (Ax +(anz*asize), Ax +(p*asize), asize) ;
                }
                anz++ ;
            }

            // get start of next vector; must be done prior to doing the
            // work of GB_jappend, since it could overwrite Ap [k+1]
            ASSERT (A->nvec <= k) ;
            p = Ap [k+1] ;

            // like GB_jappend, but always succeeds
            if (anz > anz_last)
            { 
                ASSERT (A->nvec < A->plen) ;
                Ah [A->nvec] = j ;          // add j to the pruned hyperlist
                Ap [A->nvec+1] = anz ;      // mark the end of A(:,j)
                A->nvec++ ;
            }
            anz_last = anz ;
        }

        // GB_jwrapup not needed when A is hypersparse, and A->magic already OK
        ASSERT (A->magic == GB_MAGIC) ;
        ASSERT (A->nvec <= nvec) ;

        // get the number of non-empty vectors
        anvec_new = A->nvec ;

    }
    else
    {

        //----------------------------------------------------------------------
        // prune entries from a standard matrix
        //----------------------------------------------------------------------

        // GB_for_each_vector (A): but where A->p is changing:
        for (int64_t j = 0 ; j < vdim ; j++)
        {
            // GB_for_each_entry (j, p, pend)) but A->p is changing:
            int64_t pend = Ap [j+1] ;
            for ( ; p < pend ; p++)
            {
                int64_t i = Ai [p] ;
                GB_PRUNE ;                     // test, then break or continue
                // keep this entry
                if (anz != p)
                { 
                    Ai [anz] = i ;
                    memcpy (Ax +(anz*asize), Ax +(p*asize), asize) ;
                }
                anz++ ;
            }
            p = Ap [j+1] ;                  // get start of next vector
            // like GB_jappend, but always succeeds
            Ap [j+1] = anz ;                // mark end of A (:,j)

            // count the number of non-empty vectors after pruning
            if (anz > anz_last)
            { 
                anvec_new++ ;
            }
            anz_last = anz ;
        }
        // GB_jwrapup not needed since all vectors have been traversed anyway,
        // and A->magic is already OK
        ASSERT (A->magic == GB_MAGIC) ;

    }

    // entries have now been removed; update count of non-empty vectors
    A->nvec_nonempty = anvec_new ;
    ASSERT (A->nvec_nonempty == GB_nvec_nonempty (A)) ;
}

