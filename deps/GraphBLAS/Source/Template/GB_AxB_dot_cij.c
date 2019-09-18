//------------------------------------------------------------------------------
// GB_AxB_dot_cij: compute C(i,j) = A(:,i)'*B(:,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// computes C(i,j) = A (:,i)'*B(:,j) via sparse dot product.  This template is
// used for all three cases: C=A'*B and C<!M>=A'*B in dot2, and C<M>=A'*B in
// dot3.

// GB_AxB_dot2 defines either one of these, and uses this template twice:

//      GB_PHASE_1_OF_2 ; determine if cij exists, and increment C_count
//      GB_PHASE_2_OF_2 : 2nd phase, compute cij, no realloc of C

// GB_AxB_dot3 defines GB_DOT3, and uses this template just once.

// Only one of the three are #defined: either GB_PHASE_1_OF_2, GB_PHASE_2_OF_2,
// or GB_DOT3.

#undef GB_DOT_MERGE

// cij += A(k,i) * B(k,j), for merge operation
#define GB_DOT_MERGE                                                \
{                                                                   \
    GB_GETA (aki, Ax, pA) ;             /* aki = A(k,i) */          \
    GB_GETB (bkj, Bx, pB) ;             /* bkj = B(k,j) */          \
    if (cij_exists)                                                 \
    {                                                               \
        GB_MULTADD (cij, aki, bkj) ;    /* cij += aki * bkj */      \
    }                                                               \
    else                                                            \
    {                                                               \
        /* cij = A(k,i) * B(k,j), and add to the pattern */         \
        cij_exists = true ;                                         \
        GB_MULT (cij, aki, bkj) ;       /* cij = aki * bkj */       \
    }                                                               \
}

{

    //--------------------------------------------------------------------------
    // get the start of A(:,i) and B(:,j)
    //--------------------------------------------------------------------------

    bool cij_exists = false ;   // C(i,j) not yet in the pattern
    int64_t pB = pB_start ;
    int64_t ainz = pA_end - pA ;
    ASSERT (ainz >= 0) ;

    //--------------------------------------------------------------------------
    // declare the cij scalar
    //--------------------------------------------------------------------------

    #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
    GB_CIJ_DECLARE (cij) ;
    #endif

    //--------------------------------------------------------------------------
    // compute C(i,j) = A(:,i)' * B(j,:)
    //--------------------------------------------------------------------------

    if (ainz == 0)
    { 

        //----------------------------------------------------------------------
        // A(:,i) is empty so C(i,j) cannot be present
        //----------------------------------------------------------------------

        ;

    }
    else if (Ai [pA_end-1] < ib_first || ib_last < Ai [pA])
    { 

        //----------------------------------------------------------------------
        // pattern of A(:,i) and B(:,j) do not overlap
        //----------------------------------------------------------------------

        ;

    }
    else if (bjnz == bvlen && ainz == bvlen)
    {

        //----------------------------------------------------------------------
        // both A(:,i) and B(:,j) are dense
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
        // cij = A(0,i) * B(0,j)
        GB_GETA (aki, Ax, pA) ;             // aki = A(0,i)
        GB_GETB (bkj, Bx, pB) ;             // bkj = B(0,j)
        GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj

        GB_DOT_SIMD
        for (int64_t k = 1 ; k < bvlen ; k++)
        { 
            GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
            // cij += A(k,i) * B(k,j)
            GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
            GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
            GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
        }
        #endif

    }
    else if (ainz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is dense and B(:,j) is sparse
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
        int64_t k = Bi [pB] ;               // first row index of B(:,j)
        // cij = A(k,i) * B(k,j)
        GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
        GB_GETB (bkj, Bx, pB  ) ;           // bkj = B(k,j)
        GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj

        GB_DOT_SIMD
        for (int64_t p = pB+1 ; p < pB_end ; p++)
        { 
            GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
            int64_t k = Bi [p] ;                // next row index of B(:,j)
            // cij += A(k,i) * B(k,j)
            GB_GETA (aki, Ax, pA+k) ;           // aki = A(k,i)
            GB_GETB (bkj, Bx, p   ) ;           // bkj = B(k,j)
            GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
        }
        #endif

    }
    else if (bjnz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is sparse and B(:,j) is dense
        //----------------------------------------------------------------------

        cij_exists = true ;

        #if defined ( GB_PHASE_2_OF_2 ) || defined ( GB_DOT3 )
        int64_t k = Ai [pA] ;               // first row index of A(:,i)
        // cij = A(k,i) * B(k,j)
        GB_GETA (aki, Ax, pA  ) ;           // aki = A(k,i)
        GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
        GB_MULT (cij, aki, bkj) ;           // cij = aki * bkj

        GB_DOT_SIMD
        for (int64_t p = pA+1 ; p < pA_end ; p++)
        { 
            GB_DOT_TERMINAL (cij) ;             // break if cij == terminal
            int64_t k = Ai [p] ;                // next row index of A(:,i)
            // cij += A(k,i) * B(k,j)
            GB_GETA (aki, Ax, p   ) ;           // aki = A(k,i)
            GB_GETB (bkj, Bx, pB+k) ;           // bkj = B(k,j)
            GB_MULTADD (cij, aki, bkj) ;        // cij += aki * bkj
        }
        #endif

    }
    else if (ainz > 8 * bjnz)
    {

        //----------------------------------------------------------------------
        // B(:,j) is very sparse compared to A(:,i)
        //----------------------------------------------------------------------

        while (pA < pA_end && pB < pB_end)
        {
            int64_t ia = Ai [pA] ;
            int64_t ib = Bi [pB] ;
            if (ia < ib)
            { 
                // A(ia,i) appears before B(ib,j)
                // discard all entries A(ia:ib-1,i)
                int64_t pleft = pA + 1 ;
                int64_t pright = pA_end - 1 ;
                GB_BINARY_TRIM_SEARCH (ib, Ai, pleft, pright) ;
                ASSERT (pleft > pA) ;
                pA = pleft ;
            }
            else if (ib < ia)
            { 
                // B(ib,j) appears before A(ia,i)
                pB++ ;
            }
            else // ia == ib == k
            { 
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE ;
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }

    }
    else if (bjnz > 8 * ainz)
    {

        //----------------------------------------------------------------------
        // A(:,i) is very sparse compared to B(:,j)
        //----------------------------------------------------------------------

        while (pA < pA_end && pB < pB_end)
        {
            int64_t ia = Ai [pA] ;
            int64_t ib = Bi [pB] ;
            if (ia < ib)
            { 
                // A(ia,i) appears before B(ib,j)
                pA++ ;
            }
            else if (ib < ia)
            { 
                // B(ib,j) appears before A(ia,i)
                // discard all entries B(ib:ia-1,j)
                int64_t pleft = pB + 1 ;
                int64_t pright = pB_end - 1 ;
                GB_BINARY_TRIM_SEARCH (ia, Bi, pleft, pright) ;
                ASSERT (pleft > pB) ;
                pB = pleft ;
            }
            else // ia == ib == k
            { 
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE ;
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // A(:,i) and B(:,j) have about the same sparsity
        //----------------------------------------------------------------------

        while (pA < pA_end && pB < pB_end)
        {
            int64_t ia = Ai [pA] ;
            int64_t ib = Bi [pB] ;
            if (ia < ib)
            { 
                // A(ia,i) appears before B(ib,j)
                pA++ ;
            }
            else if (ib < ia)
            { 
                // B(ib,j) appears before A(ia,i)
                pB++ ;
            }
            else // ia == ib == k
            { 
                // A(k,i) and B(k,j) are the next entries to merge
                #if defined ( GB_PHASE_1_OF_2 )
                cij_exists = true ;
                break ;
                #else
                GB_DOT_MERGE ;
                GB_DOT_TERMINAL (cij) ;         // break if cij == terminal
                pA++ ;
                pB++ ;
                #endif
            }
        }
    }

    //--------------------------------------------------------------------------
    // save C(i,j)
    //--------------------------------------------------------------------------

    #if defined ( GB_DOT3 )

        // GB_AxB_dot3: computing C<M>=A'*B
        if (cij_exists)
        { 
            // C(i,j) = cij
            GB_CIJ_SAVE (cij, pC) ;
            Ci [pC] = i ;
        }
        else
        { 
            // C(i,j) becomes a zombie
            task_nzombies++ ;
            Ci [pC] = GB_FLIP (i) ;
        }

    #else

        // GB_AxB_dot2: computing C=A'*B or C<!M>=A'*B
        if (cij_exists)
        { 
            // C(i,j) = cij
            #if defined ( GB_PHASE_1_OF_2 )
                C_count [Iter_k] ++ ;
            #else
                GB_CIJ_SAVE (cij, cnz) ;
                Ci [cnz++] = i ;
                if (cnz > cnz_last) break ;
            #endif
        }

    #endif
}

