//------------------------------------------------------------------------------
// GB_full_add_template:  phase2 for C=A+B, C<M>=A+B, C<!M>=A+B, C is full
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C is full.  The mask M is not present (otherwise, C would be sparse,
// hypersparse, or bitmap).  All of these methods are asymptotically optimal.

    //      ------------------------------------------
    //      C       =           A       +       B
    //      ------------------------------------------
    //      full    .           sparse          full  
    //      full    .           bitmap          full  
    //      full    .           full            sparse
    //      full    .           full            bitmap
    //      full    .           full            full  

// If C is iso and full, this phase has nothing to do.

#ifndef GB_ISO_ADD
{

    int64_t p ;
    ASSERT (M == NULL) ;
    ASSERT (A_is_full || B_is_full) ;
    ASSERT (C_sparsity == GxB_FULL) ;

    if (A_is_full && B_is_full)
    {

        //----------------------------------------------------------------------
        // Method30: C, A, B are all full
        //----------------------------------------------------------------------

        #pragma omp parallel for num_threads(C_nthreads) schedule(static)
        for (p = 0 ; p < cnz ; p++)
        { 
            // C (i,j) = A (i,j) + B (i,j)
            GB_LOAD_A (aij, Ax, p, A_iso) ;
            GB_LOAD_B (bij, Bx, p, B_iso) ;
            GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
        }

    }
    else if (A_is_full)
    {

        //----------------------------------------------------------------------
        // C and A are full; B is hypersparse, sparse, or bitmap
        //----------------------------------------------------------------------

        if (B_is_bitmap)
        {

            //------------------------------------------------------------------
            // Method31: C and A are full; B is bitmap
            //------------------------------------------------------------------

            #pragma omp parallel for num_threads(C_nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            {
                if (Bb [p])
                { 
                    // C (i,j) = A (i,j) + B (i,j)
                    GB_LOAD_A (aij, Ax, p, A_iso) ;
                    GB_LOAD_B (bij, Bx, p, B_iso) ;
                    GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                }
                else
                { 
                    #ifdef GB_EWISEUNION
                    { 
                        // C (i,j) = A(i,j) + beta
                        GB_LOAD_A (aij, Ax, p, A_iso) ;
                        GB_BINOP (GB_CX (p), aij, beta_scalar,
                            p % vlen, p / vlen);
                    }
                    #else
                    { 
                        // C (i,j) = A (i,j)
                        GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                    }
                    #endif
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method32: C and A are full; B is sparse or hypersparse
            //------------------------------------------------------------------

            #pragma omp parallel for num_threads(C_nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            {
                #ifdef GB_EWISEUNION
                { 
                    // C (i,j) = A(i,j) + beta
                    GB_LOAD_A (aij, Ax, p, A_iso) ;
                    GB_BINOP (GB_CX (p), aij, beta_scalar,
                        p % vlen, p / vlen) ;
                }
                #else
                { 
                    // C (i,j) = A (i,j)
                    GB_COPY_A_TO_C (GB_CX (p), Ax, p, A_iso) ;
                }
                #endif
            }

            GB_SLICE_MATRIX (B, 8, chunk) ;

            #pragma omp parallel for num_threads(B_nthreads) schedule(dynamic,1)
            for (taskid = 0 ; taskid < B_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Bslice [taskid] ;
                int64_t klast  = klast_Bslice  [taskid] ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of B(:,k) for this task
                    int64_t j = GBH (Bh, k) ;
                    int64_t pB_start, pB_end ;
                    GB_get_pA (&pB_start, &pB_end, taskid, k, kfirst,
                        klast, pstart_Bslice, Bp, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over B(:,j), the kth vector of B
                    for (int64_t pB = pB_start ; pB < pB_end ; pB++)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        int64_t i = Bi [pB] ;
                        int64_t p = pC_start + i ;
                        GB_LOAD_A (aij, Ax, p , A_iso) ;
                        GB_LOAD_B (bij, Bx, pB, B_iso) ;
                        GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                    }
                }
            }
        }

    }
    else
    {

        //----------------------------------------------------------------------
        // C and B are full; A is hypersparse, sparse, or bitmap
        //----------------------------------------------------------------------

        if (A_is_bitmap)
        {

            //------------------------------------------------------------------
            // Method33: C and B are full; A is bitmap
            //------------------------------------------------------------------

            #pragma omp parallel for num_threads(C_nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            {
                if (Ab [p])
                { 
                    // C (i,j) = A (i,j) + B (i,j)
                    GB_LOAD_A (aij, Ax, p, A_iso) ;
                    GB_LOAD_B (bij, Bx, p, B_iso) ;
                    GB_BINOP (GB_CX (p), aij, bij, p % vlen, p / vlen) ;
                }
                else
                { 
                    #ifdef GB_EWISEUNION
                    { 
                        // C (i,j) = alpha + B(i,j)
                        GB_LOAD_B (bij, Bx, p, B_iso) ;
                        GB_BINOP (GB_CX (p), alpha_scalar,
                            bij, p % vlen, p / vlen);
                    }
                    #else
                    { 
                        // C (i,j) = B (i,j)
                        GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                    }
                    #endif
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // Method34: C and B are full; A is hypersparse or sparse
            //------------------------------------------------------------------

            #pragma omp parallel for num_threads(C_nthreads) schedule(static)
            for (p = 0 ; p < cnz ; p++)
            {
                #ifdef GB_EWISEUNION
                { 
                    // C (i,j) = alpha + B(i,j)
                    GB_LOAD_B (bij, Bx, p, B_iso) ;
                    GB_BINOP (GB_CX (p), alpha_scalar, bij,
                        p % vlen, p / vlen) ;
                }
                #else
                { 
                    // C (i,j) = B (i,j)
                    GB_COPY_B_TO_C (GB_CX (p), Bx, p, B_iso) ;
                }
                #endif
            }

            GB_SLICE_MATRIX (A, 8, chunk) ;

            #pragma omp parallel for num_threads(A_nthreads) schedule(dynamic,1)
            for (taskid = 0 ; taskid < A_ntasks ; taskid++)
            {
                int64_t kfirst = kfirst_Aslice [taskid] ;
                int64_t klast  = klast_Aslice  [taskid] ;
                for (int64_t k = kfirst ; k <= klast ; k++)
                {
                    // find the part of A(:,k) for this task
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start, pA_end ;
                    GB_get_pA (&pA_start, &pA_end, taskid, k, kfirst,
                        klast, pstart_Aslice, Ap, vlen) ;
                    int64_t pC_start = j * vlen ;
                    // traverse over A(:,j), the kth vector of A
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    { 
                        // C (i,j) = A (i,j) + B (i,j)
                        int64_t i = Ai [pA] ;
                        int64_t p = pC_start + i ;
                        GB_LOAD_A (aij, Ax, pA, A_iso) ;
                        GB_LOAD_B (bij, Bx, p , B_iso) ;
                        GB_BINOP (GB_CX (p), aij, bij, i, j) ;
                    }
                }
            }
        }
    }
}
#endif

