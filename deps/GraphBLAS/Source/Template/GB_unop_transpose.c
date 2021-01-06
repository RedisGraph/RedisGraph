//------------------------------------------------------------------------------
// GB_unop_transpose: C=op(cast(A')), transpose, typecast, and apply op
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    // Ax unused for some uses of this template
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get A and C
    //--------------------------------------------------------------------------

    const GB_ATYPE *GB_RESTRICT Ax = (GB_ATYPE *) A->x ;
    GB_CTYPE *GB_RESTRICT Cx = (GB_CTYPE *) C->x ;

    //--------------------------------------------------------------------------
    // C = op (cast (A'))
    //--------------------------------------------------------------------------

    if (Workspaces == NULL)
    {

        //----------------------------------------------------------------------
        // A and C are both full or both bitmap
        //----------------------------------------------------------------------

        // A is avlen-by-avdim; C is avdim-by-avlen
        int64_t avlen = A->vlen ;
        int64_t avdim = A->vdim ;
        int64_t anz = avlen * avdim ;

        const int8_t *GB_RESTRICT Ab = A->b ;
        int8_t *GB_RESTRICT Cb = C->b ;
        ASSERT ((Cb == NULL) == (Ab == NULL)) ;

        //----------------------------------------------------------------------
        // A and C are both full or bitmap
        //----------------------------------------------------------------------

        // TODO: it would be faster to by tiles, not rows/columns, for large
        // matrices, but in most of the cases, A and C will be tall-and-thin
        // or short-and-fat.

        int tid ;
        #pragma omp parallel for num_threads(nthreads) schedule(static)
        for (tid = 0 ; tid < nthreads ; tid++)
        {
            int64_t pC_start, pC_end ;
            GB_PARTITION (pC_start, pC_end, anz, tid, nthreads) ;
            if (Ab == NULL)
            {
                // A and C are both full
                for (int64_t pC = pC_start ; pC < pC_end ; pC++)
                { 
                    // get i and j of the entry C(i,j)
                    // i = (pC % avdim) ;
                    // j = (pC / avdim) ;
                    // find the position of the entry A(j,i) 
                    // pA = j + i * avlen
                    // Cx [pC] = op (Ax [pA])
                    GB_CAST_OP (pC, ((pC / avdim) + (pC % avdim) * avlen)) ;
                }
            }
            else
            {
                // A and C are both bitmap
                for (int64_t pC = pC_start ; pC < pC_end ; pC++)
                {
                    // get i and j of the entry C(i,j)
                    // i = (pC % avdim) ;
                    // j = (pC / avdim) ;
                    // find the position of the entry A(j,i) 
                    // pA = j + i * avlen
                    int64_t pA = ((pC / avdim) + (pC % avdim) * avlen) ;
                    int8_t cij_exists = Ab [pA] ;
                    Cb [pC] = cij_exists ;
                    if (cij_exists)
                    { 
                        // Cx [pC] = op (Ax [pA])
                        GB_CAST_OP (pC, pA) ;
                    }
                }
            }
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // A is sparse or hypersparse; C is sparse
        //----------------------------------------------------------------------

        const int64_t *GB_RESTRICT Ap = A->p ;
        const int64_t *GB_RESTRICT Ah = A->h ;
        const int64_t *GB_RESTRICT Ai = A->i ;
        const int64_t anvec = A->nvec ;
        int64_t *GB_RESTRICT Ci = C->i ;

        if (nthreads == 1)
        {

            //------------------------------------------------------------------
            // sequential method
            //------------------------------------------------------------------

            int64_t *GB_RESTRICT workspace = Workspaces [0] ;
            for (int64_t k = 0 ; k < anvec ; k++)
            {
                // iterate over the entries in A(:,j)
                int64_t j = GBH (Ah, k) ;
                int64_t pA_start = Ap [k] ;
                int64_t pA_end = Ap [k+1] ;
                for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                { 
                    // C(j,i) = A(i,j)
                    int64_t i = Ai [pA] ;
                    int64_t pC = workspace [i]++ ;
                    Ci [pC] = j ;
                    // Cx [pC] = op (Ax [pA])
                    GB_CAST_OP (pC, pA) ;
                }
            }

        }
        else if (nworkspaces == 1)
        {

            //------------------------------------------------------------------
            // atomic method
            //------------------------------------------------------------------

            int64_t *GB_RESTRICT workspace = Workspaces [0] ;
            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (tid = 0 ; tid < nthreads ; tid++)
            {
                for (int64_t k = A_slice [tid] ; k < A_slice [tid+1] ; k++)
                {
                    // iterate over the entries in A(:,j)
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start = Ap [k] ;
                    int64_t pA_end = Ap [k+1] ;
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    { 
                        // C(j,i) = A(i,j)
                        int64_t i = Ai [pA] ;
                        // do this atomically:  pC = workspace [i]++
                        int64_t pC ;
                        GB_ATOMIC_CAPTURE_INC64 (pC, workspace [i]) ;
                        Ci [pC] = j ;
                        // Cx [pC] = op (Ax [pA])
                        GB_CAST_OP (pC, pA) ;
                    }
                }
            }

        }
        else
        {

            //------------------------------------------------------------------
            // non-atomic method
            //------------------------------------------------------------------

            int tid ;
            #pragma omp parallel for num_threads(nthreads) schedule(static)
            for (tid = 0 ; tid < nthreads ; tid++)
            {
                int64_t *GB_RESTRICT workspace = Workspaces [tid] ;
                for (int64_t k = A_slice [tid] ; k < A_slice [tid+1] ; k++)
                {
                    // iterate over the entries in A(:,j)
                    int64_t j = GBH (Ah, k) ;
                    int64_t pA_start = Ap [k] ;
                    int64_t pA_end = Ap [k+1] ;
                    for (int64_t pA = pA_start ; pA < pA_end ; pA++)
                    { 
                        // C(j,i) = A(i,j)
                        int64_t i = Ai [pA] ;
                        int64_t pC = workspace [i]++ ;
                        Ci [pC] = j ;
                        // Cx [pC] = op (Ax [pA])
                        GB_CAST_OP (pC, pA) ;
                    }
                }
            }
        }
    }
}

