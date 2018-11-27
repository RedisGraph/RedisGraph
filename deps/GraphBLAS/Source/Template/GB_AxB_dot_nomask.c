//------------------------------------------------------------------------------
// GB_AxB_dot_nomask:  C=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

{

    //--------------------------------------------------------------------------
    // C=A'*B via dot products
    //--------------------------------------------------------------------------

    GB_for_each_vector (B)
    {

        //----------------------------------------------------------------------
        // C(:,j) = A'*B(:,j)
        //----------------------------------------------------------------------

        int64_t GBI1_initj (Iter, j, pB_start, pB_end) ;
        int64_t bjnz = pB_end - pB_start ;
        ASSERT (bjnz >= 0) ;
        // no work to do if B(:,j) is empty
        if (bjnz == 0) continue ;
        bool B_scattered = false ;

        // get the first and last index in B(:,j)
        int64_t ib_first = Bi [pB_start] ;
        int64_t ib_last  = Bi [pB_end-1] ;

        if (A_is_hyper)
        {
            // iterate over all non-empty vectors of A
            for (int64_t ka = 0 ; ka < anvec ; ka++)
            { 
                // C(i,j) = A(:,i)'*B(:,j)
                int64_t i = Ah [ka] ;
                int64_t pA_start = Ap [ka] ;
                int64_t pA_end   = Ap [ka+1] ;
                #include "GB_cij_dot_product.c"
            }
        }
        else
        {
            // iterate over all vectors of A
            for (int64_t i = 0 ; i < anvec ; i++)
            { 
                // C(i,j) = A(:,i)'*B(:,j)
                int64_t pA_start = Ap [i] ;
                int64_t pA_end   = Ap [i+1] ;
                #include "GB_cij_dot_product.c"
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

        // cannot fail since C->plen is at the upper bound: # of non-empty
        // columns of B
        info = GB_jappend (C, j, &jlast, cnz, &cnz_last, Context) ;
        ASSERT (info == GrB_SUCCESS) ;
    }

    //--------------------------------------------------------------------------
    // finalize C
    //--------------------------------------------------------------------------

    GB_jwrapup (C, jlast, cnz) ;
}

