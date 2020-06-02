//------------------------------------------------------------------------------
// GB_dense_subassign_23_template: C += A where C is dense; A is sparse or dense
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// All entries in C+=A are computed fully in parallel, using the same kind of
// parallelism as Template/GB_AxB_colscale.c.

#include "GB_unused.h"

{

    //--------------------------------------------------------------------------
    // get C and A
    //--------------------------------------------------------------------------

    const GB_ATYPE *GB_RESTRICT Ax = A->x ;
    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    ASSERT (GB_is_dense (C)) ;

    if (kfirst_slice == NULL)
    {

        //----------------------------------------------------------------------
        // C += A when both C and A are dense
        //----------------------------------------------------------------------

        ASSERT (GB_is_dense (A)) ;
        const int64_t cnz = GB_NNZ (C) ;

        #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

            // C += A via GB_cblas_daxpy or GB_cblas_saxpy
            GB_CBLAS_AXPY           // Y += alpha*X
            (
                cnz,                // length of X and Y (note: int64_t)
                (GB_CTYPE) 1,       // alpha is 1.0
                Ax,                 // X, always stride 1
                Cx,                 // Y, always stride 1
                nthreads            // maximum # of threads to use
            ) ;

        #elif GB_HAS_CBLAS & GB_OP_IS_MINUS_REAL

            // C -= A via GB_cblas_daxpy or GB_cblas_saxpy
            GB_CBLAS_AXPY           // Y += alpha*X
            (
                cnz,                // length of X and Y (note: int64_t)
                (GB_CTYPE) -1,      // alpha is -1.0
                Ax,                 // X, always stride 1
                Cx,                 // Y, always stride 1
                nthreads            // maximum # of threads to use
            ) ;

        #else

            int64_t p ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            { 
                GB_GETB (aij, Ax, p) ;                  // aij = A(i,j)
                GB_BINOP (GB_CX (p), GB_CX (p), aij) ;  // C(i,j) += aij
            }

        #endif
    }
    else
    {

        //----------------------------------------------------------------------
        // C += A when C is dense and A is sparse
        //----------------------------------------------------------------------

        const int64_t  *GB_RESTRICT Ap = A->p ;
        const int64_t  *GB_RESTRICT Ah = A->h ;
        const int64_t  *GB_RESTRICT Ai = A->i ;
        const int64_t cvlen = C->vlen ;

        int taskid ;
        #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1)
        for (taskid = 0 ; taskid < ntasks ; taskid++)
        {

            // if kfirst > klast then taskid does no work at all
            int64_t kfirst = kfirst_slice [taskid] ;
            int64_t klast  = klast_slice  [taskid] ;

            //------------------------------------------------------------------
            // C(:,kfirst:klast) += A(:,kfirst:klast)
            //------------------------------------------------------------------

            for (int64_t k = kfirst ; k <= klast ; k++)
            {

                //--------------------------------------------------------------
                // find the part of A(:,k) and C(:,k) for this task
                //--------------------------------------------------------------

                int64_t j = (Ah == NULL) ? k : Ah [k] ;
                int64_t my_pA_start, my_pA_end ;
                GB_get_pA_and_pC (&my_pA_start, &my_pA_end, NULL,
                    taskid, k, kfirst, klast, pstart_slice, NULL, NULL, Ap) ;

                int64_t pA_start = Ap [k] ;
                bool ajdense = ((Ap [k+1] - pA_start) == cvlen) ;

                // pC points to the start of C(:,j) if C is dense
                int64_t pC = j * cvlen ;

                //--------------------------------------------------------------
                // C(:,j) += A(:,j)
                //--------------------------------------------------------------

                if (ajdense)
                { 

                    //----------------------------------------------------------
                    // both C(:,j) and A(:,j) are dense
                    //----------------------------------------------------------

                    #if GB_HAS_CBLAS & GB_OP_IS_PLUS_REAL

                        // y += x via GB_cblas_daxpy or GB_cblas_saxpy.
                        // use a single thread since this is already in a
                        // parallel region.

                        int64_t len = my_pA_end - my_pA_start ;
                        int64_t i = my_pA_start - pA_start ;
                        int64_t p = pC + i ;

                        GB_CBLAS_AXPY           // Y += alpha*X
                        (
                            len,                // length of X and Y
                            (GB_CTYPE) 1,       // alpha is 1.0
                            Ax + my_pA_start,   // X, always stride 1
                            Cx + p,             // Y, always stride 1
                            1                   // use a single thread
                        ) ;

                    #elif GB_HAS_CBLAS & GB_OP_IS_MINUS_REAL

                        // y -= x via GB_cblas_daxpy or GB_cblas_saxpy.
                        // use a single thread since this is already in a
                        // parallel region.

                        int64_t len = my_pA_end - my_pA_start ;
                        int64_t i = my_pA_start - pA_start ;
                        int64_t p = pC + i ;

                        GB_CBLAS_AXPY           // Y += alpha*X
                        (
                            len,                // length of X and Y
                            (GB_CTYPE) -1,      // alpha is -1.0
                            Ax + my_pA_start,   // X, always stride 1
                            Cx + p,             // Y, always stride 1
                            1                   // use a single thread
                        ) ;

                    #else

                        GB_PRAGMA_VECTORIZE
                        for (int64_t pA = my_pA_start ; pA < my_pA_end ; pA++)
                        { 
                            int64_t i = pA - pA_start ;
                            int64_t p = pC + i ;
                            // aij = A(i,j)
                            GB_GETB (aij, Ax, pA) ;
                            // C(i,j) += aij
                            GB_BINOP (GB_CX (p), GB_CX (p), aij) ;
                        }

                    #endif

                }
                else
                {

                    //----------------------------------------------------------
                    // C(:,j) is dense; A(:,j) is sparse 
                    //----------------------------------------------------------

                    GB_PRAGMA_VECTORIZE
                    for (int64_t pA = my_pA_start ; pA < my_pA_end ; pA++)
                    { 
                        int64_t i = Ai [pA] ;
                        int64_t p = pC + i ;
                        GB_GETB (aij, Ax, pA) ;                 // aij = A(i,j)
                        GB_BINOP (GB_CX (p), GB_CX (p), aij) ;  // C(i,j) += aij
                    }
                }
            }
        }
    }
}

