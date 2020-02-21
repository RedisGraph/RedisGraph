//------------------------------------------------------------------------------
// GB_assign_zombie1: delete all entries in C(:,j) for GB_assign
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// C(:,j)<!> = anything: GrB_Row_assign or GrB_Col_assign with an empty
// complemented mask requires all entries in the C(:,j) vector to be deleted.

#include "GB_assign.h"

void GB_assign_zombie1
(
    GrB_Matrix C,
    const int64_t j,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get C(:,j)
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Ci = C->i ;
    int64_t pC_start, pC_end, pleft = 0, pright = C->nvec-1 ;
    GB_lookup (C->is_hyper, C->h, C->p, &pleft, pright, j, &pC_start, &pC_end) ;
    int64_t cjnz = pC_end - pC_start ;
    int64_t nzombies = C->nzombies ;

    //--------------------------------------------------------------------------
    // determine the number of threads to use
    //--------------------------------------------------------------------------

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (cjnz, chunk, nthreads_max) ;

    //--------------------------------------------------------------------------
    // C(:,j) = empty
    //--------------------------------------------------------------------------

    int64_t pC ;
    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(+:nzombies)
    for (pC = pC_start ; pC < pC_end ; pC++)
    {
        int64_t i = Ci [pC] ;
        if (!GB_IS_ZOMBIE (i))
        { 
            // delete C(i,j) by marking it as a zombie
            nzombies++ ;
            Ci [pC] = GB_FLIP (i) ;
        }
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    C->nzombies = nzombies ;
}

