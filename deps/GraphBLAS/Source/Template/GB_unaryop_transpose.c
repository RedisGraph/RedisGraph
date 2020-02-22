//------------------------------------------------------------------------------
// GB_unaryop_transpose: C=op(cast(A')), transpose, typecast, and apply op
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This method is parallel, but not highly scalable.  It uses only naslice =
// nnz(A)/(A->vlen) threads.  Each thread requires O(vlen) workspace.

{

    // Ax unused for some uses of this template
    #include "GB_unused.h"

    //--------------------------------------------------------------------------
    // get A and C
    //--------------------------------------------------------------------------

    const int64_t *GB_RESTRICT Ai = A->i ;

    #if defined ( GB_PHASE_2_OF_2 )
    const GB_ATYPE *GB_RESTRICT Ax = A->x ;
    // int64_t  *GB_RESTRICT Cp = C->p ;
    int64_t  *GB_RESTRICT Ci = C->i ;
    GB_CTYPE *GB_RESTRICT Cx = C->x ;
    #endif

    //--------------------------------------------------------------------------
    // C = op (cast (A'))
    //--------------------------------------------------------------------------

    int taskid ;
    #pragma omp parallel for num_threads(naslice) schedule(static)
    for (taskid = 0 ; taskid < naslice ; taskid++)
    {
        // get the rowcount for this slice, of size A->vlen
        int64_t *GB_RESTRICT rowcount = Rowcounts [taskid] ;
        for (int64_t Iter_k = A_slice [taskid] ;
                     Iter_k < A_slice [taskid+1] ;
                     Iter_k++)
        {
            GBI_jth_iteration_with_iter (Iter, j, pA, pA_end) ;
            for ( ; pA < pA_end ; pA++)
            { 
                #if defined ( GB_PHASE_1_OF_2)
                // count one more entry in C(i,:) for this slice
                rowcount [Ai [pA]]++ ;
                #else
                // insert the entry into C(i,:) for this slice
                int64_t pC = rowcount [Ai [pA]]++ ;
                Ci [pC] = j ;
                // Cx [pC] = op (cast (Ax [pA]))
                GB_CAST_OP (pC, pA) ;
                #endif
            }
        }
    }
}

