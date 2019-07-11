//------------------------------------------------------------------------------
// GB_assign_zombie5: delete entries in C for C_replace_phase
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// For GrB_Matrix_assign, C(I,J)<M,repl>=..., if C_replace is true, and mask M
// is present, then any entry C(i,j) outside IxJ must be be deleted, if
// M(i,j)=0.

// See also GB_assign_zombie3 and GB_assign_zombie4.

#include "GB_assign.h"
#include "GB_ek_slice.h"

void GB_assign_zombie5
(
    GrB_Matrix Z,                   // the matrix C, or a copy
    const GrB_Matrix M,
    const bool Mask_comp,
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
    // get Z
    //--------------------------------------------------------------------------

    const int64_t *restrict Zh = Z->h ;
    const int64_t *restrict Zp = Z->p ;
    // const int64_t Znvec = Z->nvec ;
    int64_t *restrict Zi = Z->i ;
    int64_t nzombies = Z->nzombies ;
    const int64_t znz = GB_NNZ (Z) ;

    //--------------------------------------------------------------------------
    // get M
    //--------------------------------------------------------------------------

    const int64_t *restrict Mh = M->h ;
    const int64_t *restrict Mp = M->p ;
    const int64_t *restrict Mi = M->i ;
    const GB_void *restrict Mx = M->x ;
    const size_t msize = M->type->size ;
    const GB_cast_function cast_M =
        GB_cast_factory (GB_BOOL_code, M->type->code) ;
    const int64_t Mnvec = M->nvec ;
    const bool M_is_hyper = M->is_hyper ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (znz, chunk, nthreads_max) ;
    int ntasks = (nthreads == 1) ? 1 : (64 * nthreads) ;

    //--------------------------------------------------------------------------
    // slice the entries for each task
    //--------------------------------------------------------------------------

    // Task tid does entries pstart_slice [tid] to pstart_slice [tid+1]-1 and
    // vectors kfirst_slice [tid] to klast_slice [tid].  The first and last
    // vectors may be shared with prior slices and subsequent slices.

    int64_t pstart_slice [ntasks+1] ;
    int64_t kfirst_slice [ntasks] ;
    int64_t klast_slice  [ntasks] ;

    GB_ek_slice (pstart_slice, kfirst_slice, klast_slice, Z, ntasks) ;

    //--------------------------------------------------------------------------
    // each task creates its own zombies
    //--------------------------------------------------------------------------

    #pragma omp parallel for num_threads(nthreads) schedule(dynamic,1) \
        reduction(+:nzombies)
    for (int tid = 0 ; tid < ntasks ; tid++)
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
            // get Z(:,j) and determine if j is outside the list J
            //------------------------------------------------------------------

            int64_t j = (Zh == NULL) ? k : Zh [k] ;
            // j_outside is true if column j is outside the Z(I,J) submatrix
            bool j_outside = !GB_ij_is_in_list (J, nJ, j, Jkind, Jcolon) ;
            int64_t pZ_start, pZ_end ;
            GB_get_pA_and_pC (&pZ_start, &pZ_end, NULL,
                tid, k, kfirst, klast, pstart_slice, NULL, NULL, Zp) ;

            //------------------------------------------------------------------
            // get M(:,j)
            //------------------------------------------------------------------

            int64_t pM_start, pM_end ;
            int64_t pleft = 0 ;
            int64_t pright = Mnvec - 1 ;
            GB_lookup (M_is_hyper, Mh, Mp, &pleft, pright, j,
                &pM_start, &pM_end) ;

            //------------------------------------------------------------------
            // iterate over all entries in Z(:,j)
            //------------------------------------------------------------------

            for (int64_t pZ = pZ_start ; pZ < pZ_end ; pZ++)
            {

                //--------------------------------------------------------------
                // consider Z(i,j)
                //--------------------------------------------------------------

                // Z(i,j) is outside the Z(I,J) submatrix if either i is
                // not in the list I, or j is not in J, or both.
                int64_t i = Zi [pZ] ;
                if (!GB_IS_ZOMBIE (i) &&
                    (j_outside || !GB_ij_is_in_list (I, nI, i, Ikind, Icolon)))
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
                        cast_M (&mij, Mx +(pM*msize), 0) ;
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

