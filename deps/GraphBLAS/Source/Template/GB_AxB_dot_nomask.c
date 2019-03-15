//------------------------------------------------------------------------------
// GB_AxB_dot_nomask:  C=A'*B via dot products
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// parallel: this could be done in parallel, but the parallelism will be
// handled outside this code, in GB_AxB_parallel.  This work is done by a
// single thread.

{

    //--------------------------------------------------------------------------
    // C=A'*B via dot products
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
        // C(:,j) = A'*B(:,j)
        //----------------------------------------------------------------------

        // get the first and last index in B(:,j)
        int64_t ib_first = Bi [pB_start] ;
        int64_t ib_last  = Bi [pB_end-1] ;

        if (A_is_hyper)
        {

            //------------------------------------------------------------------
            // A is hypersparse
            //------------------------------------------------------------------

            // iterate over all non-empty vectors of A
            for (int64_t ka = 0 ; ka < anvec ; ka++)
            { 
                // get the next vector A(:,i)
                int64_t i = Ah [ka] ;
                // C(i,j) = A(:,i)'*B(:,j)
                int64_t pA     = Ap [ka] ;
                int64_t pA_end = Ap [ka+1] ;
                #include "GB_AxB_dot_cij.c"
            }
        }
        else
        {

            //------------------------------------------------------------------
            // A is non-hypersparse
            //------------------------------------------------------------------

            // iterate over all vectors of A
            for (int64_t i = 0 ; i < anvec ; i++)
            { 
                // C(i,j) = A(:,i)'*B(:,j)
                int64_t pA     = Ap [i] ;
                int64_t pA_end = Ap [i+1] ;
                #include "GB_AxB_dot_cij.c"
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

