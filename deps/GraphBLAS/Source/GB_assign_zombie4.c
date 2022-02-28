//------------------------------------------------------------------------------
// GB_assign_zombie4: delete entries in C(i,:) for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// For GrB_Row_assign or GrB_Col_assign, C(i,J)<M,repl>=..., if C_replace is
// true, and mask M is present, then any entry C(i,j) outside the list J must
// be deleted, if M(0,j)=0.

// GB_assign_zombie3 and GB_assign_zombie4 are transposes of each other.

// C must be sparse or hypersparse.
// M can have any sparsity structure: hypersparse, sparse, bitmap, or full

// C->iso is not affected.

#include "GB_assign.h"
#include "GB_assign_zombie.h"

void GB_assign_zombie4
(
    GrB_Matrix C,                   // the matrix C, or a copy
    const GrB_Matrix M,
    const bool Mask_comp,
    const bool Mask_struct,
    const int64_t i,                // index of entries to delete
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
    ASSERT (!GB_JUMBLED (C)) ;      // binary search on C
    ASSERT (!GB_PENDING (C)) ;
    ASSERT (!GB_ZOMBIES (M)) ; 
    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_PENDING (M)) ; 
    ASSERT (!GB_aliased (C, M)) ;   // NO ALIAS of C==M

    //--------------------------------------------------------------------------
    // get C
    //--------------------------------------------------------------------------

    const int64_t *restrict Ch = C->h ;
    const int64_t *restrict Cp = C->p ;
    const int64_t Cnvec = C->nvec ;
    int64_t *restrict Ci = C->i ;
    int64_t nzombies = C->nzombies ;
    const int64_t zorig = nzombies ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mh = M->h ;
    const int8_t  *restrict Mb = M->b ;
    const GB_void *restrict Mx = (GB_void *) (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const int64_t Mnvec = M->nvec ;
    const int64_t Mvlen = M->vlen ;
    ASSERT (Mvlen == 1) ;
    const bool M_is_hyper = GB_IS_HYPERSPARSE (M) ;
    const bool M_is_bitmap = GB_IS_BITMAP (M) ;
    const bool M_is_full = GB_IS_FULL (M) ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (Cnvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // delete entries in C(i,:)
    //--------------------------------------------------------------------------

    // The entry C(i,j) is deleted if j is not in the J, and if M(0,j)=0 (if
    // the mask is not complemented) or M(0,j)=1 (if the mask is complemented.

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int64_t kfirst, klast ;
        GB_PARTITION (kfirst, klast, Cnvec, taskid, ntasks) ;
        for (int64_t k = kfirst ; k < klast ; k++)
        {

            //------------------------------------------------------------------
            // get C(:,j) and determine if j is outside the list J
            //------------------------------------------------------------------

            int64_t j = GBH (Ch, k) ;
            bool j_outside = !GB_ij_is_in_list (J, nJ, j, Jkind, Jcolon) ;
            if (j_outside)
            {

                //--------------------------------------------------------------
                // j is not in J; find C(i,j)
                //--------------------------------------------------------------

                int64_t pC = Cp [k] ;
                int64_t pC_end = Cp [k+1] ;
                int64_t pright = pC_end - 1 ;
                bool found, is_zombie ;
                GB_BINARY_SEARCH_ZOMBIE (i, Ci, pC, pright, found, zorig,
                    is_zombie) ;

                //--------------------------------------------------------------
                // delete C(i,j) if found, not a zombie, and M(0,j) allows it
                //--------------------------------------------------------------

                if (found && !is_zombie)
                {

                    //----------------------------------------------------------
                    // C(i,j) is a live entry not in the C(I,J) submatrix
                    //----------------------------------------------------------

                    // Check the mask M to see if it should be deleted.
                    bool mij = false ;

                    if (M_is_bitmap || M_is_full)
                    { 
                        // M is bitmap/full, no need for GB_lookup
                        int64_t pM = j ;
                        mij = GBB (Mb, pM) && GB_mcast (Mx, pM, msize) ;
                    }
                    else
                    {
                        // M is sparse or hypersparse
                        int64_t pM, pM_end ;
                        int64_t pleft = 0 ;
                        int64_t pright = Mnvec - 1 ;
                        GB_lookup (M_is_hyper, Mh, Mp, Mvlen, &pleft, pright,
                            j, &pM, &pM_end) ;
                        if (pM < pM_end)
                        { 
                            // found it
                            mij = GB_mcast (Mx, pM, msize) ;
                        }
                    }

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

