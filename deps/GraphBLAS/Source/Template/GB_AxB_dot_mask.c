//------------------------------------------------------------------------------
// GB_AxB_dot_mask:  C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (C->vdim == M->vdim) ;
    ASSERT (C->vlen == M->vlen) ;

    //--------------------------------------------------------------------------
    // get first and last non-empty vector of A, if A is hypersparse
    //--------------------------------------------------------------------------

    int64_t ia_first = -1 ;
    int64_t ia_last  = -1 ;
    if (A_is_hyper && anvec > 0)
    { 
        // get first and last non-empty vector in A
        ia_first = Ah [0] ;
        ia_last  = Ah [anvec-1] ;
    }

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = M->x ;
    GB_cast_function cast_M = GB_cast_factory (GB_BOOL_code, M->type->code) ;
    size_t msize = M->type->size ;

    //--------------------------------------------------------------------------
    // C<M>=A'*B via dot products
    //--------------------------------------------------------------------------

    GB_for_each_vector2 (B, M)
    {

        //----------------------------------------------------------------------
        // C(:,j)<M(:,j)> = A'*B(:,j)
        //----------------------------------------------------------------------

        // vector j appears in M and/or B
        int64_t GBI2_initj (Iter, j, pB_start, pB_end, pM_start, pM_end) ;
        int64_t bjnz = pB_end - pB_start ;
        int64_t mjnz = pM_end - pM_start ;
        ASSERT (bjnz >= 0 && mjnz >= 0) ;

        // no work to do if B(:,j) or M(:,j) empty
        if (bjnz == 0 || mjnz == 0) continue ;
        bool B_scattered = false ;

        // get the first and last index in B(:,j)
        int64_t ib_first = Bi [pB_start] ;
        int64_t ib_last  = Bi [pB_end-1] ;

        // get the first and last index in M(:,j)
        int64_t im_first = Mi [pM_start] ;
        int64_t im_last  = Mi [pM_end-1] ;

        if (A_is_hyper)
        {

            //------------------------------------------------------------------
            // A is hypersparse
            //------------------------------------------------------------------

            // no work to do if the intersection of A->h and M(:,j) is empty
            if (ia_last < im_first || im_last < ia_first) continue ;

            // iterate over intersection of vectors of A and pattern of M(:,j)
            for (int64_t ka = 0, pM = pM_start ; pM < pM_end && ka < anvec ; )
            {
                // get the next entry M(i,j)
                int64_t i = Mi [pM] ;

                // get the next vector A(:,iA)
                int64_t iA = Ah [ka] ;
                if (i < iA)
                { 
                    // M(i,j) is present but A(:,iA) is empty
                    // C(i,j) cannot be present
                    pM++ ;
                    continue ;
                }
                else if (i > iA)
                { 
                    // A(:,i) is non-empty but M(i,j) is not present
                    // C(i,j) cannot be present
                    ka++ ;
                    continue ;
                }

                // C(i,j) might be present; M(i,j) is present and both
                // A(:,i) and B(:,j) are not empty. check M(i,j).
                bool mij ;
                cast_M (&mij, Mx +(pM*msize), 0) ;
                if (mij)
                { 
                    // C(i,j) = A(:,i)*B(:,j)
                    int64_t pA_start = Ap [ka] ;
                    int64_t pA_end   = Ap [ka+1] ;
                    #include "GB_cij_dot_product.c"
                }

                ASSERT (Mi [pM] == Ah [ka]) ;
                ka++ ;  // advance to next non empty vector of A
                pM++ ;  // advance to the next index in M(:,j)
            }

        }
        else
        {

            //------------------------------------------------------------------
            // A is non-hypersparse
            //------------------------------------------------------------------

            // iterate over all entries in M(:,j)
            for (int64_t pM = pM_start ; pM < pM_end ; pM++)
            {
                // get the next entry M(i,j)
                int64_t i = Mi [pM] ;
                bool mij ;
                cast_M (&mij, Mx +(pM*msize), 0) ;
                if (mij)
                { 
                    // C(i,j) = A(:,i)'*B(:,j)
                    int64_t pA_start = Ap [i] ;
                    int64_t pA_end   = Ap [i+1] ;
                    #include "GB_cij_dot_product.c"
                }
            }
        }

        //----------------------------------------------------------------------
        // clear workspace if B(:,j) was scattered
        //----------------------------------------------------------------------

        if (B_scattered)
        {
            for (int64_t pB = pB_start ; pB < pB_end ; pB++)
            { 
                Flag [Bi [pB]] = 0 ;
            }
            B_scattered = false ;
        }

        //----------------------------------------------------------------------
        // log the end of C(:,j)
        //----------------------------------------------------------------------

        // cannot fail since C->plen is the upper bound: # of non-empty columns
        // of B
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;
        ASSERT (info == GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // finalize construction of C
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;
}

