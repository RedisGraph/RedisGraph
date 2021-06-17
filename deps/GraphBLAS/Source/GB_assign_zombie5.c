//------------------------------------------------------------------------------
// GB_assign_zombie5: delete entries in C for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// For GrB_Matrix_assign, C(I,J)<M,repl>=..., if C_replace is true, and mask M
// is present, then any entry C(i,j) outside IxJ must be be deleted, if
// M(i,j)=0.

// See also GB_assign_zombie3 and GB_assign_zombie4.

// C must be sparse or hypersparse.

#include "GB_assign.h"
#include "GB_assign_zombie.h"
#include "GB_subassign_methods.h"
#include "GB_ek_slice.h"

#undef  GB_FREE_WORK
#define GB_FREE_WORK \
    GB_ek_slice_free (&pstart_slice, &kfirst_slice, &klast_slice) ;
#undef  GB_FREE_ALL
#define GB_FREE_ALL GB_FREE_WORK

GrB_Info GB_assign_zombie5
(
    GrB_Matrix C,                   // the matrix C, or a copy
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
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
    // get C
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ch = C->h ;
    const int64_t *GB_RESTRICT Cp = C->p ;
    // const int64_t Cnvec = C->nvec ;
    int64_t *GB_RESTRICT Ci = C->i ;
    int64_t nzombies = C->nzombies ;
    const int64_t zvlen = C->vlen ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t *GB_RESTRICT Mh = M->h ;
    const int8_t  *GB_RESTRICT Mb = M->b ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const int64_t Mnvec = M->nvec ;
    const int64_t Mvlen = M->vlen ;
    const bool M_is_hyper = GB_IS_HYPERSPARSE (M) ;
    const bool M_is_bitmap = GB_IS_BITMAP (M) ;
    const bool M_is_full = GB_IS_FULL (M) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    const int64_t znz = GB_NNZ_HELD (C) ;
    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (znz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t *pstart_slice = NULL, *kfirst_slice = NULL, *klast_slice = NULL ;
    if (!GB_ek_slice (&pstart_slice, &kfirst_slice, &klast_slice, C, &ntasks))
    { 
        // out of memory
        return (GrB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // each task creates its own zombies
    //--------------------------------------------------------------------------

    int tid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (tid = 0 ; tid < ntasks ; tid++)
    {

        //----------------------------------------------------------------------
        // get the task description
        //----------------------------------------------------------------------

        int64_t kfirst = kfirst_slice [tid] ;
        int64_t klast  = klast_slice  [tid] ;

        //----------------------------------------------------------------------
        // scan vectors kfirst to klast for entries to delete
        //----------------------------------------------------------------------

        for (int64_t k = kfirst ; k <= klast ; k++)
        {

            //------------------------------------------------------------------
            // get C(:,j) and determine if j is outside the list J
            //------------------------------------------------------------------

            int64_t j = GBH (Ch, k) ;
            // j_outside is true if column j is outside the C(I,J) submatrix
            bool j_outside = !GB_ij_is_in_list (J, nJ, j, Jkind, Jcolon) ;
            int64_t pC_start, pC_end ;
            GB_get_pA (&pC_start, &pC_end, tid, k,
                kfirst, klast, pstart_slice, Cp, zvlen) ;

            //------------------------------------------------------------------
            // get M(:,j)
            //------------------------------------------------------------------

            // this works for M with any sparsity structure
            int64_t pM_start, pM_end ;
            int64_t pright = Mnvec - 1 ;
            int64_t pleft = 0 ;
            GB_lookup (M_is_hyper, Mh, Mp, Mvlen, &pleft, pright, j,
                &pM_start, &pM_end) ;
            bool mjdense = (pM_end - pM_start) == Mvlen ;

            //------------------------------------------------------------------
            // iterate over all entries in C(:,j)
            //------------------------------------------------------------------

            for (int64_t pC = pC_start ; pC < pC_end ; pC++)
            {

                //--------------------------------------------------------------
                // consider C(i,j)
                //--------------------------------------------------------------

                // C(i,j) is outside the C(I,J) submatrix if either i is
                // not in the list I, or j is not in J, or both.
                int64_t i = Ci [pC] ;
                if (!GB_IS_ZOMBIE (i) &&
                    (j_outside || !GB_ij_is_in_list (I, nI, i, Ikind, Icolon)))
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
    // free workspace and return result
    //--------------------------------------------------------------------------

    C->nzombies = nzombies ;
    GB_FREE_WORK ;
    return (GrB_SUCCESS) ;
}

