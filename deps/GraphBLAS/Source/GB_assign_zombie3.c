//------------------------------------------------------------------------------
// GB_assign_zombie3: delete entries in C(:,j) for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// For GrB_Row_assign or GrB_Col_assign, C(I,j)<#M,repl>=any must delete all
// entries C(i,j) outside of C(I,j), if the mask M(i,0) (or its complement) is
// zero.  This step is not done for GxB_*_subassign, since that method does not
// modify anything outside IxJ.

// GB_assign_zombie3 and GB_assign_zombie4 are transposes of each other.

// C must be sparse or hypersparse.
// M can have any sparsity structure: hypersparse, sparse, bitmap, or full

// C->iso is not affected.

#include "GB_assign.h"
#include "GB_assign_zombie.h"
#include "GB_subassign_methods.h"

void GB_assign_zombie3
(
    GrB_Matrix C,                   // the matrix C, or a copy
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,
    const int64_t j,                // vector index with entries to delete
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (!GB_IS_FULL (C)) ;
    ASSERT (!GB_IS_BITMAP (C)) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_ZOMBIES (M)) ; 
    ASSERT (!GB_JUMBLED (M)) ;      // binary search on M
    ASSERT (!GB_PENDING (M)) ; 
    ASSERT (!GB_aliased (C, M)) ;   // NO ALIAS of C==M

    //--------------------------------------------------------------------------
    // get C (:,j)
    //--------------------------------------------------------------------------

    const int64_t *restrict Ch = C->h ;
    const int64_t *restrict Cp = C->p ;
    int64_t *restrict Ci = C->i ;
    int64_t pC_start, pC_end, pleft = 0, pright = C->nvec-1 ;
    GB_lookup (C->h != NULL, Ch, Cp, C->vlen, &pleft, pright, j,
        &pC_start, &pC_end) ;
    int64_t nzombies = C->nzombies ;
    const int64_t zjnz = pC_end - pC_start ;

    //--------------------------------------------------------------------------
    // get M(:,0)
    //--------------------------------------------------------------------------

    const int64_t *restrict Mp = M->p ;
    const int8_t  *restrict Mb = M->b ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const int64_t Mvlen = M->vlen ;
    int64_t pM_start = 0 ; // Mp [0]
    int64_t pM_end = GBP (Mp, 1, Mvlen) ;
    const bool M_is_bitmap = GB_IS_BITMAP (M) ;
    const bool mjdense = (pM_end - pM_start) == Mvlen ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (zjnz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // delete entries from C(:,j) that are outside I, if the mask M allows it
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int64_t p1, p2 ;
        GB_PARTITION (p1, p2, zjnz, taskid, ntasks) ;
        for (int64_t pC = pC_start + p1 ; pC < pC_start + p2 ; pC++)
        {

            //------------------------------------------------------------------
            // get C(i,j)
            //------------------------------------------------------------------

            int64_t i = Ci [pC] ;
            if (!GB_IS_ZOMBIE (i))
            {

                //--------------------------------------------------------------
                // C(i,j) is outside C(I,j) if i is not in the list I
                //--------------------------------------------------------------

                bool i_outside = !GB_ij_is_in_list (I, nI, i, Ikind, Icolon) ;
                if (i_outside)
                {

                    //----------------------------------------------------------
                    // C(i,j) is a live entry not in the C(I,J) submatrix
                    //----------------------------------------------------------

                    // Check the mask M to see if it should be deleted.
                    GB_MIJ_BINARY_SEARCH_OR_DENSE_LOOKUP (i) ;
                    if (Mask_comp)
                    { 
                        // negate the mask if Mask_comp is true
                        mij = !mij ;
                    }
                    if (!mij)
                    { 
                        // delete C(i,j) by marking it as a zombie
                        nzombies++ ;
                        Ci [pC] = GB_FLIP (i) ;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    C->nzombies = nzombies ;
}

