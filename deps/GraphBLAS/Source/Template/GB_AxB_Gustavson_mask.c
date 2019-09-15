//------------------------------------------------------------------------------
// GB_AxB_Gustavson_mask:  compute C<M>=A*B using the Gustavson method, with M
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd in GB_AxB_Gustavson.c, and Template/GB_AxB.c, the
// latter of which expands into Generated/GB_AxB__* for all built-in semirings.

// The pattern of C has not been computed, but nnz(M) has given an upper bound
// on nnz(C) so this method will not run out of memory.  This is Gustavson's
// method, extended to handle hypersparse matrices, arbitrary semirings, and a
// mask matrix M.

// The mask is present in this case (see GB_AxB_Gustavson_nomask otherwise).
// This method takes Omega(nnz(M)) time when exploiting the mask, so a very
// dense mask can be costly to exploit.  Thus, this method is not used, and
// GB_AxB_Gustavson_nomask is used instead, if the total flop count is less
// than nnz(M).

{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_aliased (C, M)) ;
    ASSERT (!GB_aliased (C, A)) ;
    ASSERT (!GB_aliased (C, B)) ;
    ASSERT (C->vdim == B->vdim) ;
    ASSERT (C->vlen == A->vlen) ;
    ASSERT (A->vdim == B->vlen) ;
    ASSERT (C->vdim == M->vdim) ;
    ASSERT (C->vlen == M->vlen) ;

    //--------------------------------------------------------------------------
    // get the Sauna
    //--------------------------------------------------------------------------

    // clear Sauna_Mark and ensure hiwater+1 does not cause integer overflow
    int64_t *restrict Sauna_Mark = Sauna->Sauna_Mark ;
    int64_t hiwater = GB_Sauna_reset (Sauna, 1, 1) ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = M->x ;
    GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;
    size_t msize = M->type->size ;
    #ifdef GB_HYPER_CASE
    const int64_t *restrict Mh = M->h ;
    const int64_t mnvec = M->nvec ;
    int64_t mpleft = 0 ;
    int64_t mpright = mnvec - 1 ;
    #endif

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const int64_t *restrict Bi = B->i ;

    #ifdef GB_HYPER_CASE
    const int64_t *restrict Ah = A->h ;
    int64_t anvec = A->nvec ;
    int64_t pleft = 0, pright = anvec-1 ;
    #endif

    //--------------------------------------------------------------------------
    // start the construction of C
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;
    #ifndef GB_HYPER_CASE
    int64_t *restrict Cp = C->p ;
    #endif

    int64_t jlast, cnz, cnz_last ;
    GB_jstartup (C, &jlast, &cnz, &cnz_last) ;

    //--------------------------------------------------------------------------
    // C<M>=A*B using the Gustavson method, pattern of C is a subset of M
    //--------------------------------------------------------------------------

    GBI_for_each_vector (B)
    {

        //----------------------------------------------------------------------
        // get B(:,j)
        //----------------------------------------------------------------------

        GBI_jth_iteration (j, pB, pB_end) ;

        //----------------------------------------------------------------------
        // get M(:,j)
        //----------------------------------------------------------------------

        // find vector j in M
        int64_t pM_start, pM_end ;
        #ifdef GB_HYPER_CASE
        GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j, &pM_start, &pM_end);
        #else
        pM_start = Mp [j] ;
        pM_end   = Mp [j+1] ;
        #endif
        ASSERT (pM_start <= pM_end) ;
        ASSERT (pM_end >= -1) ;

        // C(:,j) is empty if either M(:,j) or B(:,j) are empty
        int64_t bjnz = pB_end - pB ;
        if (pM_start == pM_end || bjnz == 0)
        { 
            #ifndef GB_HYPER_CASE
            Cp [j+1] = cnz ;
            #endif
            continue ;
        }

        // M(:,j) has at least one entry; get the first and last index in M(:,j)
        int64_t im_first = Mi [pM_start] ;
        int64_t im_last  = Mi [pM_end-1] ;

        #ifdef GB_HYPER_CASE
        // trim Ah on right
        if (A_is_hyper)
        {
            pleft = 0 ;
            pright = anvec-1 ;
            if (bjnz > 2)
            { 
                // trim Ah [0..pright] to remove any entries past last B(:,j)
                int64_t klast = Bi [pB_end-1] ;
                GB_bracket_right (klast, Ah, 0, &pright) ;
            }
        }
        #endif

        // M(:,j) is not yet scattered into Sauna_Mark
        bool marked = false ;

        //----------------------------------------------------------------------
        // C(:,j)<M(:,j)> = A * B(:,j), both values and pattern
        //----------------------------------------------------------------------

        for ( ; pB < pB_end ; pB++)
        {

            //------------------------------------------------------------------
            // get the pattern of B(k,j)
            //------------------------------------------------------------------

            int64_t k = Bi [pB] ;

            //------------------------------------------------------------------
            // get A(:,k)
            //------------------------------------------------------------------

            // find A(:,k), reusing pleft since Bi [...] is sorted
            int64_t pA, pA_end ;
            #ifdef GB_HYPER_CASE
            GB_lookup (A_is_hyper, Ah, Ap, &pleft, pright, k, &pA, &pA_end) ;
            #else
            pA     = Ap [k] ;
            pA_end = Ap [k+1] ;
            #endif

            // skip if A(:,k) is empty
            if (pA == pA_end) continue ;

            // skip if the intersection of A(:,k) and M(:,j) is empty
            if (Ai [pA_end-1] < im_first || Ai [pA] > im_last) continue ;

            //------------------------------------------------------------------
            // scatter M(:,j) into Sauna_Mark if not yet done
            //------------------------------------------------------------------

            if (!marked)
            {
                for (int64_t pM = pM_start ; pM < pM_end ; pM++)
                {
                    // mij = (bool) M (i,j)
                    bool mij ;
                    cast_M (&mij, Mx +(pM*msize), 0) ;
                    if (mij)
                    { 
                        // M(i,j) is true
                        Sauna_Mark [Mi [pM]] = hiwater ;
                    }
                }
                // M(:,j) has been scattered into Sauna_Mark
                marked = true ;

                // status of Sauna_Mark [0..cvlen-1]:
                // Sauna_Mark [i] < hiwater: M(i,j)=0, or not present in M(:,j)
                // Sauna_Mark [i] = hiwater: M(i,j) = 1
            }

            //------------------------------------------------------------------
            // get the value of B(k,j)
            //------------------------------------------------------------------

            GB_GETB (bkj, Bx, pB) ;

            //------------------------------------------------------------------
            // Sauna += (A(:,k) * B(k,j)) .* M(:,j)
            //------------------------------------------------------------------

            for ( ; pA < pA_end ; pA++)
            {
                // Sauna_Work [i] += (A(i,k) * B(k,j)) .* M(i,j)
                int64_t i = Ai [pA] ;
                int64_t mark = Sauna_Mark [i] ;
                if (mark < hiwater) continue ;
                // M(i,j) == 1 so do the work
                GB_GETA (aik, Ax, pA) ;

                // Sauna_Work [i] += A(i,k) * B(k,j)
                if (mark == hiwater)
                { 
                    // first time C(i,j) seen
                    // Sauna_Work [i] = A(i,k) * B(k,j)
                    GB_MULT (GB_SAUNA_WORK (i), aik, bkj) ;
                    Sauna_Mark [i] = hiwater + 1 ;
                }
                else
                { 
                    // C(i,j) seen before, update it
                    // Sauna_Work [i] += A(i,k) * B(k,j)
                    GB_MULTADD (GB_SAUNA_WORK (i), aik, bkj) ;
                }
            }

            //------------------------------------------------------------------
            // status of Sauna_Mark [0..cvlen-1] and Sauna_Work [0..cvlen-1]
            //------------------------------------------------------------------

            // Sauna_Mark [i] < hiwater:   M(i,j)=0, or not present in M(:,j)
            // Sauna_Mark [i] = hiwater:   M(i,j)=1, C(i,j) not present;
            //                             Sauna_Work [i] uninitialized
            // Sauna_Mark [i] = hiwater+1: M(i,j)=1, and C(i,j) is present;
            //                             value is Sauna_Work [i]
        }

        //----------------------------------------------------------------------
        // check if C(:,j) is empty
        //----------------------------------------------------------------------

        // if M(:,j) has not been scattered into Sauna_Mark, then C(:,j) must be
        // empty.  C(:,j) can still be empty if marked is false, but in that
        // case the Sauna_Mark must still be cleared.

        #ifdef GB_HYPER_CASE
        if (!marked) continue ;
        #endif

        //----------------------------------------------------------------------
        // gather C(:,j), from pattern of M(:,j) and values in Sauna_Work
        //----------------------------------------------------------------------

        if (marked)
        {
            for (int64_t pM = pM_start ; pM < pM_end ; pM++)
            {
                int64_t i = Mi [pM] ;
                if (Sauna_Mark [i] == hiwater+1)
                { 
                    // C(i,j) is a live entry, gather its row and value
                    // Cx [cnz] = Sauna_Work [i] ;
                    ASSERT (cnz < C->nzmax) ;
                    GB_COPY_C (GB_CX (cnz), GB_SAUNA_WORK (i)) ;
                    Ci [cnz++] = i ;
                }
            }

            // clear the Sauna_Mark by incrementing hiwater by 2, and ensuring
            // that the resulting hiwater+1 does not cause integer overflow
            hiwater = GB_Sauna_reset (Sauna, 2, 1) ;
        }

        //----------------------------------------------------------------------
        // log the end of C(:,j)
        //----------------------------------------------------------------------

        #ifdef GB_HYPER_CASE
        // cannot fail since C->plen is the upper bound: number of non-empty
        // columns of B
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, NULL) ;
        ASSERT (info == GrB_SUCCESS) ;
        #else
        Cp [j+1] = cnz ;
        if (cnz > cnz_last) C->nvec_nonempty++ ;
        cnz_last = cnz ;
        #endif
    }

    //--------------------------------------------------------------------------
    // finalize C
    //--------------------------------------------------------------------------

    #ifdef GB_HYPER_CASE
    GB_jwrapup (C, jlast, cnz) ;
    ASSERT (info == GrB_SUCCESS) ;
    #else
    C->magic = GB_MAGIC ;
    #endif
}

