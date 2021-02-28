//------------------------------------------------------------------------------
// GB_assign_zombie4: delete entries in C(i,:) for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// For GrB_Row_assign or GrB_Col_assign, C(i,J)<M,repl>=..., if C_replace is
// true, and mask M is present, then any entry C(i,j) outside the list J must
// be deleted, if M(0,j)=0.

// GB_assign_zombie3 and GB_assign_zombie4 are transposes of each other.

#include "GB_assign.h"

void GB_assign_zombie4
(
    GrB_Matrix Z,                   // the matrix C, or a copy
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
    // get Z
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Zh = Z->h ;
    const int64_t *GB_RESTRICT Zp = Z->p ;
    const int64_t Znvec = Z->nvec ;
    int64_t *GB_RESTRICT Zi = Z->i ;
    int64_t nzombies = Z->nzombies ;
    const int64_t zorig = nzombies ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Mh = M->h ;
    const int64_t *GB_RESTRICT Mp = M->p ;
    const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    const int64_t Mnvec = M->nvec ;
    const bool M_is_hyper = M->is_hyper ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (Znvec, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // delete entries in Z(i,:)
    //--------------------------------------------------------------------------

    // The entry Z(i,j) is deleted if j is not in the J, and if M(0,j)=0 (if
    // the mask is not complemented) or M(0.j)=1 (if the mask is complemented.

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int64_t kfirst, klast ;
        GB_PARTITION (kfirst, klast, Znvec, taskid, ntasks) ;
        for (int64_t k = kfirst ; k < klast ; k++)
        {

            //------------------------------------------------------------------
            // get Z(:,j) and determine if j is outside the list J
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            bool j_outside = !GB_ij_is_in_list (J, nJ, j, Jkind, Jcolon) ;
            if (j_outside)
            {

                //--------------------------------------------------------------
                // j is not in J; find Z(i,j)
                //--------------------------------------------------------------

                int64_t pZ = Zp [k] ;
                int64_t pZ_end = Zp [k+1] ;
                int64_t pright = pZ_end - 1 ;
                bool found, is_zombie ;
                GB_BINARY_SEARCH_ZOMBIE (i, Zi, pZ, pright, found, zorig,
                    is_zombie) ;

                //--------------------------------------------------------------
                // delete Z(i,j) if found, not a zombie, and M(0,j) allows it
                //--------------------------------------------------------------

                if (found && !is_zombie)
                {

                    //----------------------------------------------------------
                    // Z(i,j) is a live entry not in the Z(I,J) submatrix
                    //----------------------------------------------------------

                    // Check the mask M to see if it should be deleted.

                    int64_t pM, pM_end ;
                    int64_t pleft = 0 ;
                    int64_t pright = Mnvec - 1 ;
                    GB_lookup (M_is_hyper, Mh, Mp, &pleft, pright, j,
                        &pM, &pM_end) ;
                    bool mij = false ;
                    if (pM < pM_end)
                    { 
                        // found it
                        mij = GB_mcast (Mx, pM, msize) ;
                    }
                    if (Mask_comp)
                    { 
                        // negate the mask if Mask_comp is true
                        mij = !mij ;
                    }
                    if (!mij)
                    { 
                        // delete Z(i,j) by marking it as a zombie
                        nzombies++ ;
                        Zi [pZ] = GB_FLIP (i) ;
                    }
                }
            }
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    Z->nzombies = nzombies ;
}

