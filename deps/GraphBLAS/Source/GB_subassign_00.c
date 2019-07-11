//------------------------------------------------------------------------------
// GB_subassign_00: C(I,J)<!,repl> = empty ; using S
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Method 00: C(I,J)<!,repl> = empty ; using S

// M:           NULL
// Mask_comp:   true
// C_replace:   true
// accum:       any (present or not; result is the same)
// A:           any (scalar or matrix; result is the same)
// S:           constructed

#include "GB_subassign_methods.h"

GrB_Info GB_subassign_00
(
    GrB_Matrix C,
    // input:
    const GrB_Index *I,
    const int64_t nI,
    const int Ikind,
    const int64_t Icolon [3],
    const GrB_Index *J,
    const int64_t nJ,
    const int Jkind,
    const int64_t Jcolon [3],
    const GrB_Matrix S,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // get inputs
    //--------------------------------------------------------------------------

    int64_t *restrict Ci = C->i ;
    const int64_t *restrict Sx = S->x ;

    //--------------------------------------------------------------------------
    // Method 00: C(I,J)<!,repl> = empty ; using S
    //--------------------------------------------------------------------------

    // Time: Optimal, O(nnz(S)), assuming S has already been constructed.

    //--------------------------------------------------------------------------
    // Parallel: all entries in S can be processed fully in parallel.
    //--------------------------------------------------------------------------

    // All entries in C(I,J) are deleted.  The result does not depend on A or
    // the scalar.

    int64_t snz = GB_NNZ (S) ;

    GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
    int nthreads = GB_nthreads (snz, chunk, nthreads_max) ;

    int64_t task_nzombies = C->nzombies ;

    #pragma omp parallel for num_threads(nthreads) schedule(static) \
        reduction(+:task_nzombies)
    for (int64_t pS = 0 ; pS < snz ; pS++)
    { 
        // S (inew,jnew) is a pointer back into C (I(inew), J(jnew))
        GB_C_S_LOOKUP ;
        // ----[X A 0] or [X . 0]-----------------------------------------------
        // action: ( X ): still a zombie
        // ----[C A 0] or [C . 0]-----------------------------------------------
        // action: C_repl: ( delete ): becomes a zombie
        GB_DELETE_ENTRY ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    C->nzombies = task_nzombies ;
    return (GrB_SUCCESS) ;
}

