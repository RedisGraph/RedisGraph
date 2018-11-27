//------------------------------------------------------------------------------
// GB_cij_dot_product: compute C(i,j) = A(:,i)'*B(:,j)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// computes C(i,j) = A (:,i)'*B(:,j) via sparse dot product, optionally
// scattering B(:,j) into size-bvlen workspace.

// FUTURE: some add operators can terminate their outer loop early.  For
// example, if the add monoid is the logical OR, then the outer loop can
// terminate as soon as cij is true.  If the add operator is FIRST, of any
// type, then it can terminate immediately after the first cij = assignment.
// If the add operator is SECOND, of any type, then the loop to accumuate cij
// could be down backwards, and the loop would terminate after the first
// assignment.

// cij += A(k,i) * B(k,j)
#define GB_DOT_MULTADD(pA,pB)                                   \
    GB_DOT_GETA (pA) ;         /* aki = A(k,i) */               \
    GB_DOT_GETB (pB) ;         /* bkj = B(k,j) */               \
    GB_DOT_MULT (bkj) ;        /* t = aki * bkj */              \
    GB_DOT_ADD ;               /* cij += t */

// cij = t (and flag cij as existing) or cij += t
#define GB_DOT_ACCUM \
{                                                               \
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
}

// cij += A(k,i) * B(k,j), for merge operation
#define GB_DOT_MERGE                                            \
{                                                               \
    GB_DOT_GETA (pA++) ;       /* aki = A(k,i) */               \
    GB_DOT_GETB (pB++) ;       /* bkj = B(k,j) */               \
    GB_DOT_MULT (bkj) ;        /* t = aki * bkj */              \
    GB_DOT_ACCUM ;             /* cij = t or += t */            \
}

{

    //--------------------------------------------------------------------------
    // get the start of A(:,i) and B(:,j)
    //--------------------------------------------------------------------------

    bool cij_exists = false ;   // C(i,j) not yet in the pattern
    int64_t pA = pA_start ;
    int64_t pB = pB_start ;
    int64_t ainz = pA_end - pA_start ;
    ASSERT (ainz >= 0) ;

    //--------------------------------------------------------------------------
    // ensure enough space exists in C
    //--------------------------------------------------------------------------

    if (cnz == C->nzmax)
    {
        GrB_Info info = GB_ix_realloc (C, 2*(C->nzmax), true, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (Chandle) ;
            GB_DOT_FREE_WORK ;
            return (info) ;
        }
        Ci = C->i ;
        Cx = C->x ;
        // reacquire cij since C->x has moved
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
    else if (Ai [pA_end-1] < ib_first || ib_last < Ai [pA_start])
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
    else if (B_can_scatter)
    { 

        //----------------------------------------------------------------------
        // scatter B(:,j) into size-bvlen workspace
        //----------------------------------------------------------------------

        if (Flag == NULL)
        {
            // allocate Flag and Work space of size bvlen
            GB_CALLOC_MEMORY (Flag, bvlen, sizeof (int8_t)) ;
            GB_MALLOC_MEMORY (Work, bvlen, bkj_size) ;
            if (Flag == NULL || Work == NULL)
            { 
                // out of memory
                double memory = GBYTES (bvlen, sizeof (int8_t)) +
                                GBYTES (bvlen, bkj_size) ;
                GB_MATRIX_FREE (Chandle) ;
                GB_DOT_FREE_WORK ;
                return (GB_OUT_OF_MEMORY (memory)) ;
            }
        }

        if (!B_scattered)
        {
            // scatter B into the workspace
            for ( ; pB < pB_end ; pB++)
            { 
                int64_t k = Bi [pB] ;
                // Work [k] = Bx [pB] ;
                GB_DOT_SCATTER ;
                Flag [k] = 1 ;
            }
            B_scattered = true ;
        }

        for ( ; pA < pA_end ; pA++)
        {
            int64_t k = Ai [pA] ;
            if (Flag [k])
            { 
                // cij += A(k,i) * Work [k], where Work [k] == B(k,j)
                GB_DOT_GETA (pA) ;              // aki = A (k,i)
                GB_DOT_MULT (GB_DOT_WORK(k)) ;  // t = aki * Work [k]
                GB_DOT_ACCUM ;                  // cij = t or += t
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

#undef GB_DOT_MULTADD
#undef GB_DOT_MERGE
#undef GB_DOT_ACCUM

