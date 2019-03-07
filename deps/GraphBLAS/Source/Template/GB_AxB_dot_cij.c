//------------------------------------------------------------------------------
// GB_AxB_dot_cij: compute C(i,j) = A(:,i)'*B(:,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// computes C(i,j) = A (:,i)'*B(:,j) via sparse dot product.

// parallel: the parallelism will be handled outside this code, in
// GB_AxB_parallel.  This work is done by a single thread.

#undef GB_DOT_MULTADD
#undef GB_DOT_MERGE

// cij += A(k,i) * B(k,j)
#define GB_DOT_MULTADD(pA,pB)                                   \
{                                                               \
    GB_DOT_GETA (pA) ;         /* aki = A(k,i) */               \
    GB_DOT_GETB (pB) ;         /* bkj = B(k,j) */               \
    GB_DOT_MULT (bkj) ;        /* t = aki * bkj */              \
    GB_DOT_ADD ;               /* cij += t */                   \
    GB_DOT_TERMINAL (cij) ;    /* break if cij == terminal */   \
}

// cij += A(k,i) * B(k,j), for merge operation
#define GB_DOT_MERGE                                            \
{                                                               \
    GB_DOT_GETA (pA++) ;       /* aki = A(k,i) */               \
    GB_DOT_GETB (pB++) ;       /* bkj = B(k,j) */               \
    GB_DOT_MULT (bkj) ;        /* t = aki * bkj */              \
    if (cij_exists)                                             \
    {                                                           \
        GB_DOT_ADD ;           /* cij += t */                   \
    }                                                           \
    else                                                        \
    {                                                           \
        /* cij = A(k,i) * B(k,j), and add to the pattern */     \
        cij_exists = true ;                                     \
        GB_DOT_COPY ;          /* cij = t */                    \
    }                                                           \
    GB_DOT_TERMINAL (cij) ;    /* break if cij == terminal */   \
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
    // ensure enough space exists in C
    //--------------------------------------------------------------------------

    if (cnz == C->nzmax)
    {
        GrB_Info info = GB_ix_realloc (C, 2*(C->nzmax), true, NULL) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            ASSERT (!(C->enqueued)) ;
            GB_free (Chandle) ;
            return (info) ;
        }
        Ci = C->i ;
        Cx = C->x ;
        // reacquire the pointer cij C->x has moved
        GB_DOT_REACQUIRE ;
    }

    //--------------------------------------------------------------------------
    // compute C(i,j)
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
        GB_DOT_CLEAR ;                         // cij = identity
        for (int64_t k = 0 ; k < bvlen ; k++)
        { 
            GB_DOT_MULTADD (pA+k, pB+k) ;      // cij += A(k,i) * B(k,j)
        }

    }
    else if (ainz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is dense and B(:,j) is sparse
        //----------------------------------------------------------------------

        cij_exists = true ;
        GB_DOT_CLEAR ;                         // cij = identity
        for ( ; pB < pB_end ; pB++)
        { 
            int64_t k = Bi [pB] ;
            GB_DOT_MULTADD (pA+k, pB) ;        // cij += A(k,i) * B(k,j)
        }

    }
    else if (bjnz == bvlen)
    {

        //----------------------------------------------------------------------
        // A(:,i) is sparse and B(:,j) is dense
        //----------------------------------------------------------------------

        cij_exists = true ;
        GB_DOT_CLEAR ;                         // cij = identity
        for ( ; pA < pA_end ; pA++)
        { 
            int64_t k = Ai [pA] ;
            GB_DOT_MULTADD (pA, pB+k) ;        // cij += A(k,i) * B(k,j)
        }

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
                GB_DOT_MERGE ;
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
                GB_DOT_MERGE ;
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
                GB_DOT_MERGE ;
            }
        }
    }

    //--------------------------------------------------------------------------
    // save C(i,j)
    //--------------------------------------------------------------------------

    if (cij_exists)
    { 
        // C(i,j) = cij
        GB_DOT_SAVE ;
        Ci [cnz++] = i ;
    }
}

