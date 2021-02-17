//------------------------------------------------------------------------------
// GB_AxB_saxpy3_coarseHash_phase5:
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

{

    //--------------------------------------------------------------------------
    // phase 5: coarse hash task, C=A*B
    //--------------------------------------------------------------------------

    // Initially, Hf [...] < mark for all of Hf.
    // Let f = Hf [hash] and h = Hi [hash]

    // f < mark          : unoccupied.
    // h == i, f == mark : occupied with C(i,j)

    for (int64_t kk = kfirst ; kk <= klast ; kk++)
    {
        int64_t pC = Cp [kk] ;
        int64_t cjnz = Cp [kk+1] - pC ;
        if (cjnz == 0) continue ;   // nothing to do
        GB_GET_B_j ;                // get B(:,j)

        #ifdef GB_CHECK_MASK_ij

            // The mask M is packed (full, bitmap, or sparse/hyper and not
            // jumbled with all entries present in the entire matrix).  Get
            // pointers Mjb and Mjx into the M(:,j) vector.
            GB_GET_M_j                  // get M(:,j)
            #ifndef M_SIZE
            #define M_SIZE 1
            #endif
            const M_TYPE *GB_RESTRICT Mjx = Mask_struct ? NULL :
                ((M_TYPE *) Mx) + (M_SIZE * pM_start) ;
            const int8_t *GB_RESTRICT Mjb = M_is_bitmap ? (Mb+pM_start) : NULL ;

        #else

            // M is not present
            if (bjnz == 1 && (A_is_sparse || A_is_hyper))
            { 
                // C(:,j) = A(:,k)*B(k,j), no mask
                GB_COMPUTE_C_j_WHEN_NNZ_B_j_IS_ONE ;
                continue ;
            }

        #endif

        mark++ ;
        for ( ; pB < pB_end ; pB++)     // scan B(:,j)
        {
            GB_GET_B_kj_INDEX ;         // get index k of B(k,j)
            GB_GET_A_k ;                // get A(:,k)
            if (aknz == 0) continue ;
            GB_GET_B_kj ;               // bkj = B(k,j)
            // scan A(:,k)
            for (int64_t pA = pA_start ; pA < pA_end ; pA++)
            {
                GB_GET_A_ik_INDEX ;     // get index i of A(i,j)
                #ifdef GB_CHECK_MASK_ij
                // check mask condition and skip if C(i,j) is protected by the
                // mask
                GB_CHECK_MASK_ij ;
                #endif
                GB_MULT_A_ik_B_kj ;     // t = A(i,k)*B(k,j)
                for (GB_HASH (i))   // find i in hash table
                {
                    if (Hf [hash] == mark)
                    {
                        // hash entry is occupied
                        if (Hi [hash] == i)
                        { 
                            // i already in the hash table
                            // Hx [hash] += t ;
                            GB_HX_UPDATE (hash, t) ;
                            break ;
                        }
                    }
                    else
                    { 
                        // hash entry is not occupied
                        Hf [hash] = mark ;
                        Hi [hash] = i ;
                        GB_HX_WRITE (hash, t) ;// Hx[hash]=t
                        Ci [pC++] = i ;
                        break ;
                    }
                }
            }
        }
        GB_SORT_AND_GATHER_HASHED_C_j (mark) ;  // gather into C(:,j)
    }
}

#undef M_TYPE
#undef M_SIZE

