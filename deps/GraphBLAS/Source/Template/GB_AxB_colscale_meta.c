//------------------------------------------------------------------------------
// GB_AxB_colscale_meta: C=A*D where D is a square diagonal matrix
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All entries in C=A*D are computed fully in parallel, using the same kind of
// parallelism as Template/GB_reduce_each_vector.c.

{

    // Dx, j, and Ah are unused if the operator is FIRST or PAIR
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get C, A, and D
    //--------------------------------------------------------------------------

    const int64_t  *GB_RESTRICT Ap = A->p ;
    const int64_t  *GB_RESTRICT Ah = A->h ;
    const GB_ATYPE *GB_RESTRICT Ax = A_is_pattern ? NULL : A->x ;
    const GB_BTYPE *GB_RESTRICT Dx = D_is_pattern ? NULL : D->x ;

    //--------------------------------------------------------------------------
    // C=A*D
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        // if kfirst > klast then task tid does no work at all
        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        //----------------------------------------------------------------------
        // C(:,kfirst:klast) = A(:,kfirst:klast)*D(kfirst:klast,kfirst:klast)
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // find the part of A(:,k) and C(:,k) to be operated on by this task
            //------------------------------------------------------------------

            int64_t j = (Ah == NULL) ? k : Ah [k] ;
            int64_t pA_start, pA_end ;
            GB_get_pA_and_pC (&pA_start, &pA_end, NULL,
                tid, k, kfirst, klast, pstart_slice, NULL, NULL, Ap) ;

            //------------------------------------------------------------------
            // C(:,j) = A(:,j)*D(j,j)
            //------------------------------------------------------------------

            GB_GETB (djj, Dx, j) ;                  // djj = D (j,j)
            GB_PRAGMA_VECTORIZE
            for (int64_t p = pA_start ; p < pA_end ; p++)
            { 
                GB_GETA (aij, Ax, p) ;              // aij = A(i,j)
                GB_BINOP (GB_CX (p), aij, djj) ;    // C(i,j) = aij * djj
            }
        }
    }
}

