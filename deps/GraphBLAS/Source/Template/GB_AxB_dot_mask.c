//------------------------------------------------------------------------------
// GB_AxB_dot_mask:  C<M>=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// parallel: this could be done in parallel, but the parallelism will be
// handled outside this code, in GB_AxB_parallel.  This work is done by a
// single thread.

{

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
    // C<M>=A'*B via dot products
    //--------------------------------------------------------------------------

    GBI_for_each_vector (B)
    {

        //----------------------------------------------------------------------
        // get B(:,j)
        //----------------------------------------------------------------------

        GBI_jth_iteration (j, pB_start, pB_end) ;
        int64_t bjnz = pB_end - pB_start ;
        // no work to do if B(:,j) is empty
        if (bjnz == 0) continue ;

        //----------------------------------------------------------------------
        // get M(:,j)
        //----------------------------------------------------------------------

        // find vector j in M
        int64_t pM, pM_end ;
        GB_lookup (M_is_hyper, Mh, Mp, &mpleft, mpright, j, &pM, &pM_end) ;
        // no work to do if M(:,j) is empty
        if (pM == pM_end) continue ;

        //----------------------------------------------------------------------
        // C(:,j)<M(:,j)> = A'*B(:,j)
        //----------------------------------------------------------------------

        // get the first and last index in B(:,j)
        int64_t ib_first = Bi [pB_start] ;
        int64_t ib_last  = Bi [pB_end-1] ;

        // get the first and last index in M(:,j)
        int64_t im_first = Mi [pM] ;
        int64_t im_last  = Mi [pM_end-1] ;

        if (A_is_hyper)
        {

            //------------------------------------------------------------------
            // A is hypersparse
            //------------------------------------------------------------------

            // no work to do if the intersection of A->h and M(:,j) is empty
            if (ia_last < im_first || im_last < ia_first) continue ;

            // FUTURE::  if Ah is small and nnz(M(:,j)) is large, then iterate
            // through Ah and use a binary search into M(:,j).  Likewise,
            // if the reverse is true, iterate through M(:,j); if M(i,j)
            // is true, then use GB_lookup to find A(:,i), and then do
            // the dot product.

            // iterate over intersection of vectors of A and pattern of M(:,j)
            for (int64_t ka = 0 ; pM < pM_end && ka < anvec ; )
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
                    // C(i,j) = A(:,i)'*B(:,j)
                    int64_t pA     = Ap [ka] ;
                    int64_t pA_end = Ap [ka+1] ;
                    #include "GB_AxB_dot_cij.c"
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

            // No binary search is needed since every vector is present in A.

            // iterate over all entries in M(:,j)
            for ( ; pM < pM_end ; pM++)
            {
                
                // get the next entry M(i,j)
                int64_t i = Mi [pM] ;
                bool mij ;
                cast_M (&mij, Mx +(pM*msize), 0) ;
                if (mij)
                { 
                    // C(i,j) = A(:,i)'*B(:,j)
                    int64_t pA     = Ap [i] ;
                    int64_t pA_end = Ap [i+1] ;
                    #include "GB_AxB_dot_cij.c"
                }
            }
        }

        //----------------------------------------------------------------------
        // log the end of C(:,j)
        //----------------------------------------------------------------------

        // cannot fail since C->plen is at the upper bound: # of non-empty
        // columns of B
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, NULL) ;
        ASSERT (info == GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // finalize C
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;
}

