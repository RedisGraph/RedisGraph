//------------------------------------------------------------------------------
// GB_assign_zombie3: delete entries in C(:,j) for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// For GrB_Row_assign or GrB_Col_assign, C(I,j)<#M,repl>=any must delete all
// entries C(i,j) outside of C(I,j), if the mask M(i,0) (or its complement) is
// zero.  This step is not done for GxB_*_subassign, since that method does not
// modify anything outside IxJ.

// GB_assign_zombie3 and GB_assign_zombie4 are transposes of each other.

#include "GB_assign.h"

void GB_assign_zombie3
(
    GrB_Matrix Z,                   // the matrix C, or a copy
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
    // get Z (:,j)
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Zh = Z->h ;
    const int64_t *GB_RESTRICT Zp = Z->p ;
    int64_t *GB_RESTRICT Zi = Z->i ;
    int64_t pZ_start, pZ_end, pleft = 0, pright = Z->nvec-1 ;
    GB_lookup (Z->is_hyper, Zh, Zp, &pleft, pright, j, &pZ_start, &pZ_end) ;
    int64_t nzombies = Z->nzombies ;
    const int64_t zjnz = pZ_end - pZ_start ;

    //--------------------------------------------------------------------------
    // get M(:,0)
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Mp = M->p ;
    const int64_t *GB_RESTRICT Mi = M->i ;
    const GB_void *GB_RESTRICT Mx = (Mask_struct ? NULL : (M->x)) ;
    const size_t msize = M->type->size ;
    int64_t pM_start = Mp [0] ;
    int64_t pM_end = Mp [1] ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (zjnz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // delete entries from Z(:,j) that are outside I, if the mask M allows it
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (taskid = 0 ; taskid < ntasks ; taskid++)
    {
        int64_t p1, p2 ;
        GB_PARTITION (p1, p2, zjnz, taskid, ntasks) ;
        for (int64_t pZ = pZ_start + p1 ; pZ < pZ_start + p2 ; pZ++)
        {

            //------------------------------------------------------------------
            // get Z(i,j)
            //------------------------------------------------------------------

            int64_t i = Zi [pZ] ;
            if (!GB_IS_ZOMBIE (i))
            {

                //--------------------------------------------------------------
                // Z(i,j) is outside Z(I,j) if i is not in the list I
                //--------------------------------------------------------------

                bool i_outside = !GB_ij_is_in_list (I, nI, i, Ikind, Icolon) ;
                if (i_outside)
                {

                    //----------------------------------------------------------
                    // Z(i,j) is a live entry not in the Z(I,J) submatrix
                    //----------------------------------------------------------

                    // Check the mask M to see if it should be deleted.

                    int64_t pM     = pM_start ;
                    int64_t pright = pM_end - 1 ;
                    bool found ;
                    GB_BINARY_SEARCH (i, Mi, pM, pright, found) ;
                    bool mij = false ;
                    if (found)
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

