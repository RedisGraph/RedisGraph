//------------------------------------------------------------------------------
// GB_AxB_Gustavson_nomask:  C=A*B using Gustavson method, precomputed pattern
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This file is #include'd in GB_AxB_Gustavson.c, and Template/GB_AxB.c, the
// latter of which expands into Generated/GB_AxB__* for all built-in semirings.

// The pattern of C has already been computed in the symbolic phase of
// GB_AxB_Gustavson.  This is Gustavson's method, extended to handle
// hypersparse matrices and arbitrary semirings.

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
    ASSERT (Sauna->Sauna_n >= C->vlen) ;

    //--------------------------------------------------------------------------
    // get A and B
    //--------------------------------------------------------------------------

    const int64_t *restrict Ap = A->p ;
    const int64_t *restrict Ai = A->i ;
    const int64_t *restrict Bi = B->i ;

    #ifdef GB_HYPER_CASE
    const int64_t *restrict Ah = A->h ;
    int64_t anvec = A->nvec ;
    #endif

    //--------------------------------------------------------------------------
    // get C (pattern already constructed)
    //--------------------------------------------------------------------------

    const int64_t *restrict Ci = C->i ;
    const int64_t *restrict Cp = C->p ;
    ASSERT (C->nvec <= B->nvec) ;

    #ifdef GB_HYPER_CASE
    const int64_t *restrict Ch = C->h ;
    int64_t cnvec = C->nvec ;
    int64_t kc = 0 ;
    #endif

    // C->p and C->h have already been computed in the symbolic phase
    ASSERT (C->magic == GB_MAGIC) ;

    //--------------------------------------------------------------------------
    // C=A*B using the Gustavson's saxpy-based method; precomputed pattern of C
    //--------------------------------------------------------------------------

    GBI_for_each_vector (B)
    {

        //----------------------------------------------------------------------
        // get B(:,j)
        //----------------------------------------------------------------------

        GBI_jth_iteration (j, pB, pB_end) ;
        int64_t bjnz = pB_end - pB ;
        // no work to do if B(:,j) is empty
        if (bjnz == 0) continue ;

        //----------------------------------------------------------------------
        // get C(:,j)
        //----------------------------------------------------------------------

        int64_t pC_start, pC_end ;
        #ifdef GB_HYPER_CASE
        if (C_is_hyper)
        {
            // C will have a subset of the columns of B, so do a linear-time
            // search for j in Ch.  The total time for this search is just
            // O(cnvec), for the entire matrix multiply.  No need for a binary
            // search using GB_lookup.
            bool found = false ;
            for ( ; kc < cnvec && Ch [kc] <= j ; kc++)
            {
                found = (Ch [kc] == j) ;
                if (found)
                { 
                    pC_start = Cp [kc] ;
                    pC_end   = Cp [kc+1] ;
                    break ;
                }
            }
            // skip if C (:,j) is empty
            if (!found) continue ;
        }
        else
        #endif
        { 
            pC_start = Cp [j] ;
            pC_end   = Cp [j+1] ;
        }

        // skip if C(:,j) is empty
        if (pC_end == pC_start) continue ;

        //----------------------------------------------------------------------
        // clear Sauna_Work
        //----------------------------------------------------------------------

        for (int64_t pC = pC_start ; pC < pC_end ; pC++)
        { 
            // Sauna_Work [Ci [pC]] = identity ;
            GB_COPY_C (GB_SAUNA_WORK (Ci [pC]), GB_IDENTITY) ;
        }

        #ifdef GB_HYPER_CASE
        // trim Ah on right
        int64_t pleft = 0 ;
        int64_t pright = anvec-1 ;
        if (A_is_hyper && bjnz > 2)
        { 
            // trim Ah [0..pright] to remove any entries past the last B(:,j)
            GB_bracket_right (Bi [pB_end-1], Ah, 0, &pright) ;
        }
        #endif

        //----------------------------------------------------------------------
        // C(:,j) = A * B(:,j)
        //----------------------------------------------------------------------

        for ( ; pB < pB_end ; pB++)
        {

            //------------------------------------------------------------------
            // get B(k,j) and A(:,k)
            //------------------------------------------------------------------

            // get the pattern of B(k,j)
            int64_t k = Bi [pB] ;

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

            // get the value of B(k,j)
            // bkj = Bx [pB]
            GB_GETB (bkj, Bx, pB) ;

            //------------------------------------------------------------------
            // Sauna_Work += A(:,k) * B(k,j)
            //------------------------------------------------------------------

            for ( ; pA < pA_end ; pA++)
            { 
                // Sauna_Work [i] += A(i,k) * B(k,j)
                int64_t i = Ai [pA] ;
                GB_GETA (aik, Ax, pA) ;
                GB_MULTADD (GB_SAUNA_WORK (i), aik, bkj) ;
            }
        }

        //----------------------------------------------------------------------
        // gather C(:,j) from Sauna_Work
        //----------------------------------------------------------------------

        for (int64_t pC = pC_start ; pC < pC_end ; pC++)
        { 
            // Cx [pC] = Sauna_Work [Ci [pC]] ;
            GB_COPY_C (GB_CX (pC), GB_SAUNA_WORK (Ci [pC])) ;
        }
    }
}

