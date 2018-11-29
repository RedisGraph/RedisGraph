//------------------------------------------------------------------------------
// GB_AxB_heap_mask:  compute C<M>=A*B using the heap method, with M present
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

// This file is #include'd in GB_AxB_heap.c, and Template/GB_AxB.c, the
// latter of which expands into Generated/GB_AxB__* for all built-in semirings.

// if GB_MASK_CASE is defined, then the mask matrix M is present.  Otherwise it
// is not present.

#ifndef GB_HEAP_FREE_WORK
#define GB_HEAP_FREE_WORK
#endif

{

    //--------------------------------------------------------------------------
    // get the mask
    //--------------------------------------------------------------------------

    #ifdef GB_MASK_CASE
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = M->x ;
    GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;
    size_t msize = M->type->size ;
    #endif

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    const int64_t *restrict Ah = A->h ;
    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const int64_t *restrict Bi = B->i ;
    int64_t anvec = A->nvec ;

    // if A is hypersparse but all vectors are present, then
    // treat A as if it were non-hypersparse
    bool A_is_hyper = A->is_hyper && (anvec < A->vdim) ;

    //--------------------------------------------------------------------------
    // start the construction of C
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    //--------------------------------------------------------------------------
    // C<M> = A*B
    //--------------------------------------------------------------------------

    #ifdef GB_MASK_CASE
    GB_for_each_vector2 (B, M)
    #else
    GB_for_each_vector (B)
    #endif
    {

        //----------------------------------------------------------------------
        // get B(:,j) and M(:,j)
        //----------------------------------------------------------------------

        #ifdef GB_MASK_CASE
        int64_t GBI2_initj (Iter, j, pB_start, pB_end, pM, pM_end) ;
        #else
        int64_t GBI1_initj (Iter, j, pB_start, pB_end) ;
        #endif

        // C(:,j) is empty if either M(:,j) or B(:,j) are empty
        int64_t bjnz = pB_end - pB_start ;
        if (bjnz == 0) continue ;

        #ifdef GB_MASK_CASE
        int64_t mjnz = pM_end - pM ;
        if (mjnz == 0) continue ;

        // M(:,j) has at least one entry; get the first and last index in M(:,j)
        int64_t im_first = Mi [pM] ;
        int64_t im_last  = Mi [pM_end-1] ;
        #endif

        //----------------------------------------------------------------------
        // trim Ah on right
        //----------------------------------------------------------------------

        // Ah [0..A->nvec-1] holds the set of non-empty vectors of A, but only
        // vectors k corresponding to nonzero entries B(k,j) are accessed for
        // this vector B(:,j).  If nnz (B(:,j)) > 2, prune the search space on
        // the right, so the remaining calls to GB_lookup will only need to
        // search Ah [pleft...pright-1].  pright does not change.  pleft is
        // advanced as the Heap is built, since the indices in B(:,j) are
        // sorted in ascending order.

        int64_t pleft = 0 ;
        int64_t pright = anvec-1 ;
        if (A_is_hyper && bjnz > 2)
        { 
            // trim Ah [0..pright] to remove any entries past the last B(:,j)
            GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;
        }

        //----------------------------------------------------------------------
        // build the Heap
        //----------------------------------------------------------------------

        // Construct a Heap containing each vector A(:,k) for which B(k,j) is
        // nonzero.

        // The key of an GB_Element in the Heap is the index i at the head of
        // the A(:,k) list.  The name each Elemens is the corresponding entry
        // in B(:,j) with index k.  The name is kk if B(k,j) is the (kk)th
        // nonzero in B(:,j), if kk is in the range 0 to bjnz-1.

        // each array is of size bjnz_max
        // int64_t pA_pair [0..bjnz-1] ;
        // GB_Element Heap [1..bjnz] ;     note that Heap [0] is not valid
        // int64_t List [0..bjnz-1] ;
        int64_t nheap = 0 ;
        ASSERT (bjnz <= bjnz_max) ;

        for (int64_t pB = pB_start ; pB < pB_end ; pB++)
        { 
            // B(k,j) is nonzero
            int64_t k = Bi [pB] ;

            // find A(:,k), reusing pleft since Bi [...] is sorted
            int64_t pA, pA_end ;
            GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, k, &pA, &pA_end) ;

            // skip if A(:,k) empty
            if (pA == pA_end) continue ;

            #ifdef GB_MASK_CASE
            // A(:,k) is non-empty; get the first and last index of A(:,k)
            int64_t alo = Ai [pA] ;
            int64_t ahi = Ai [pA_end-1] ;

            // skip if the intersection of A(:,k) and M(:,j) is empty
            if (ahi < im_first || alo > im_last) continue ;

            // skip past all rows in A(:,k) that are < im_first
            for ( ; pA < pA_end && Ai [pA] < im_first ; pA++) ;

            // skip if remainder of A(:,k) is empty
            if (pA == pA_end) continue ;
            #endif

            // A(:,k) not empty; add the first index in A(:,k) to the heap
            ++nheap ;

            // k is renamed kk.  B(k,j) is the kk-th nonzero in B(:,j)
            int64_t kk = pB - pB_start ;
            ASSERT (kk >= 0 && kk < bjnz) ;
            Heap [nheap].key  = Ai [pA] ;
            Heap [nheap].name = kk ;

            // keep track of the start and end of A(:,k)
            pA_pair [kk].start = pA ;
            pA_pair [kk].end   = pA_end ;
        }

        GB_heap_build (Heap, nheap) ;

        // keep track of the number live nodes in the Heap
        int64_t nlive = nheap ;

        //----------------------------------------------------------------------
        // C (:,j) = A (:,K) * B(K,j), for all indices K = find (B (:,j))
        //----------------------------------------------------------------------

        #ifdef GB_MASK_CASE
        while (nheap > 0)   // iterate until all A(:,k) are done
        #else
        while (nheap > 1)   // iterate until only one A(:,k) is left
        #endif
        {

            //------------------------------------------------------------------
            // C(i,j) = A (i,List)' * B (List,j)
            //------------------------------------------------------------------

            // The method scans the Heap to find all nodes with minimum key.
            // This key is the row index i.  Each of these nodes represents a
            // vector A(:,k) for which the topmost row index is A(i,k). The
            // nodes are placed in a List, in topological order.  Next, a dot
            // product is computed, cij = A (i,List)' * B (List,j), for all
            // nodes in the List.  As each node is processed, its key is
            // updated to the next row index in the vector, and the node is
            // reheapified.  The reheapify is done in reverse topological order
            // so that the min-heap property is preserved.

            //------------------------------------------------------------------
            // get the List of all nodes with minimum key
            //------------------------------------------------------------------

            int64_t nlist ;
            int64_t i = GB_heap_getminlist (Heap, nheap, List, &nlist) ;
            ASSERT (i >= 0 && i < cvlen) ;

            //------------------------------------------------------------------
            // get the mask M(i,j)
            //------------------------------------------------------------------

            #ifdef GB_MASK_CASE
            // get M(i,j) and advance the mask
            for ( ; pM < pM_end && Mi [pM] < i ; pM++) ;
            if (pM >= pM_end)
            { 
                // M(:,j) is exhausted; C(:,j) is done
                nheap = 0 ;
                break ;
            }
            bool mij = false ;
            if (i == Mi [pM])
            { 
                cast_M (&mij, Mx +(pM*msize), 0) ;
                pM++ ;
            }
            int64_t im_next = (pM < pM_end) ? Mi [pM] : cvlen ;
            #endif

            //------------------------------------------------------------------
            // ensure enough space exists in C
            //------------------------------------------------------------------

            #ifdef GB_MASK_CASE
            // C->nzmax == nnz (M) + 1, so cnz < C->nzmax will always hold
            ASSERT (cnz < C->nzmax) ;
            #else
            {
                // ensure enough space exists in C
                if (cnz == C->nzmax)
                {
                    GrB_Info info = GB_ix_realloc (C, 2*(C->nzmax), true,
                        Context) ;
                    if (info != GrB_SUCCESS)
                    { 
                        // out of memory
                        GB_MATRIX_FREE (Chandle) ;
                        GB_HEAP_FREE_WORK ;
                        return (info) ;
                    }
                    Ci = C->i ;
                    Cx = C->x ;
                    // reacquire cij since C->x has moved
                    GB_CIJ_REACQUIRE ;
                }
            }
            #endif

            //------------------------------------------------------------------
            // cij = 0
            //------------------------------------------------------------------

            #ifdef GB_MASK_CASE
            if (mij)
            #endif
            {
                // cij = 0
                GB_CIJ_CLEAR ;
            }

            //------------------------------------------------------------------
            // cij = A (i,List)' * B (List,j), in topological order
            //------------------------------------------------------------------

            for (int64_t t = nlist-1 ; t >= 0 ; t--)
            {

                //--------------------------------------------------------------
                // get node p from the List, which defines A(:,k)
                //--------------------------------------------------------------

                // the index k is implicit; it has been renamed as kk if
                // B(k,j) is the kk-th nonzero in B(:,j).
                // get k from the list and the position of A(i,k)
                int64_t p = List [t] ;          ASSERT (p >= 1 && p <= nheap) ;
                int64_t kk = Heap [p].name ;    ASSERT (kk >= 0 && kk < bjnz) ;
                int64_t pA     = pA_pair [kk].start ;
                int64_t pA_end = pA_pair [kk].end ;
                ASSERT (Ai [pA] == i) ;

                //--------------------------------------------------------------
                // C(i,j) += A(i,k) * B(k,j)
                //--------------------------------------------------------------

                // This is a dot product of A(i,:)' and B(:,j), but unlike the
                // dot product method, this loop cannot terminate early for
                // operators such as logical OR, or FIRST.  Each vector A(:,k)
                // must be advanced in the Heap.  The numerical work, below,
                // could be skipped, but this is trivial for built-in
                // operators.  Early termination cannot be exploited for
                // user-defined semirings since their properties are unknown.

                #ifdef GB_MASK_CASE
                if (mij)
                #endif
                {
                    // cij += A(i,k) * B(k,j)
                    GB_CIJ_MULTADD (pA, pB_start + kk) ;
                }

                //--------------------------------------------------------------
                // move to the next entry in A(:,k)
                //--------------------------------------------------------------

                #ifdef GB_MASK_CASE
                // skip past all rows in A(:,k) that are < im_next
                if (im_next == cvlen)
                { 
                    // M(:,j) is exhausted and thus A(:,j) is done too
                    pA = pA_end ;
                }
                else
                { 
                    for ( ; pA < pA_end && Ai [pA] < im_next ; pA++) ;
                }
                #else
                // advance to the next row in A(:,k)
                pA++ ;
                #endif

                //--------------------------------------------------------------
                // put A(:,k) back in the Heap
                //--------------------------------------------------------------

                pA_pair [kk].start = pA ;
                if (pA < pA_end)
                { 
                    // advance p to the next entry in A(:,k) or M(:,j).
                    // kk < bjnz refers to A(:,k), and kk=bjnz is M(:,j).
                    Heap [p].key = Ai [pA] ;
                    ASSERT (Heap [p].key > i && Heap [p].key < cvlen) ;
                    GB_heapify (p, Heap, nheap) ;
                }
                else
                {
                    // A(:,k) is exhausted.  Either delete it from the Heap
                    // if safe to do so, or give it a terminal key.
                    ASSERT (nheap > 0) ;
                    if (Heap [nheap].key > i)
                    { 
                        // safe to delete p from the Heap
                        GB_heap_delete (p, Heap, &nheap) ;
                    }
                    else
                    { 
                        // Heap [nheap].key == i, so the last node in the
                        // Heap is an entry in the List [0..nlist-1] that
                        // has not yet been processed in this for-loop.  It
                        // is not safe to delete.  Give node p a terminal
                        // key so and heapify it is no longer considered.
                        Heap [p].key = cvlen ;
                        GB_heapify (p, Heap, nheap) ;
                    }
                    // one less live node in the Heap
                    --nlive ;
                }
            }

            //------------------------------------------------------------------
            // prune the Heap if mostly dead
            //------------------------------------------------------------------

            ASSERT (GB_IMPLIES (nheap > 0, nlive <= nheap && nlive >= 0)) ;

            if (nlive == 0)
            { 
                // nothing is left
                nheap = 0 ;
            }

            if (nheap > 0 && 2*nlive < nheap)
            {
                // less than half of the Heap is alive.  Prune the dead.
                // This step also ensures that Heap [1].key is never == cvlen,
                // since that would mean nlive == 0 and nheap > 0.
                int64_t nheap_pruned = 0 ;
                for (int64_t p = 1 ; p <= nheap ; p++)
                {
                    if (Heap [p].key < cvlen)
                    { 
                        // keep this element in the Heap
                        Heap [++nheap_pruned] = Heap [p] ;
                    }
                }
                ASSERT (nheap_pruned == nlive) ;
                nheap = nheap_pruned ;
                nlive = nheap ;

                // rebuild the Heap
                GB_heap_build (Heap, nheap) ;
            }

            //------------------------------------------------------------------
            // insert C(i,j) into C
            //------------------------------------------------------------------

            #ifdef GB_MASK_CASE
            if (mij)
            #endif
            { 
                Ci [cnz] = i ;
                // Cx [cnz] = cij ;
                GB_CIJ_SAVE ;
                cnz++ ;
            }
        }

        //----------------------------------------------------------------------
        // handle the last A(:,k)
        //----------------------------------------------------------------------

        // This phase is done only if the mask is not present.  If the mask is
        // present, the while loop above terminates only when the Heap is
        // empty.

        #ifndef GB_MASK_CASE
        if (nheap == 1)
        {
            // get the last A(:,k)
            int64_t ilast = Heap [1].key ;
            ASSERT (ilast >= 0 && ilast < cvlen) ;
            int64_t kk = Heap [1].name ;
            ASSERT (kk >= 0 && kk < bjnz) ;

            int64_t pA     = pA_pair [kk].start ;
            int64_t pA_end = pA_pair [kk].end ;
            ASSERT (ilast == Ai [pA]) ;

            // number of entries left in this last A(:,k)
            int64_t aknz = pA_end - pA ;

            // ensure enough space exists in C
            if (cnz + aknz > C->nzmax)
            {
                GrB_Info info = GB_ix_realloc (C, 2*(cnz + aknz), true,
                    Context) ;
                if (info != GrB_SUCCESS)
                { 
                    // out of memory
                    GB_MATRIX_FREE (Chandle) ;
                    GB_HEAP_FREE_WORK ;
                    return (info) ;
                }
                Ci = C->i ;
                Cx = C->x ;
                // reacquire cij since C->x has moved
                GB_CIJ_REACQUIRE ;
            }

            // bkj = Bx [] ;
            GB_CIJ_GETB (pB_start + kk) ;

            // C(ilast:end,j) = A (ilast:end,k) * B (k,j)
            for ( ; pA < pA_end ; pA++)
            { 
                // get A(i,k) and B(k,j) and do the numerical work
                int64_t i = Ai [pA] ;

                // cij = A(i,k) * B(k,j)
                GB_CIJ_MULT (pA) ;

                Ci [cnz] = i ;
                // Cx [cnz] = cij ;
                GB_CIJ_SAVE ;
                cnz++ ;
            }
        }
        #endif

        //----------------------------------------------------------------------
        // log the end of vector C(:,j)
        //----------------------------------------------------------------------

        // this cannot fail since C->plen is the upper bound: the number
        // of non-empty vectors of B.
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;
        ASSERT (info == GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // finish construction of C
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;    // finalize Cp and Ch
}

