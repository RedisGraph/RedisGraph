//------------------------------------------------------------------------------
// GB_add_phase1: find # of entries in C=A+B or C<M>=A+B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_add_phase1 counts the number of entries in each vector of C, for C=A+B or
// C<M>=A+B  and then does a cumulative sum to find Cp.  GB_add_phase1 is
// preceded by GB_add_phase0, which finds the non-empty vectors of C.  This
// phase is done entirely in parallel.

// C, M, A, and B can be standard sparse or hypersparse, as determined by
// GB_add_phase0.  The mask M may be present, but it is not complemented.

// GB_wait computes A=A+T where T is the matrix of the assembled pending
// tuples.  A and T are disjoint, so this function does not need to examine
// the pattern of A and T at all.  No mask is used in this case.

// Cp is either freed by phase2, or transplanted into C.

#include "GB_add.h"

GrB_Info GB_add_phase1                  // count nnz in each C(:,j)
(
    int64_t **Cp_handle,                // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    const bool A_and_B_are_disjoint,    // if true, then A and B are disjoint
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,      // array of structs
    const int ntasks,                       // # of tasks
    const int nthreads,                     // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
    const bool Ch_is_Mh,                // if true, then Ch == M->h
    // original input:
    const GrB_Matrix M,                 // optional mask, may be NULL
    const bool Mask_struct,         // if true, use the only structure of M
    const GrB_Matrix A,
    const GrB_Matrix B,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cp_handle != NULL) ;
    ASSERT (Cnvec_nonempty != NULL) ;
    ASSERT_MATRIX_OK (A, "A for add phase1", GB0) ;
    ASSERT_MATRIX_OK (B, "B for add phase1", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for add phase1", GB0) ;
    ASSERT (A->vdim == B->vdim) ;

    int64_t *GB_RESTRICT Cp = NULL ;
    (*Cp_handle) = NULL ;

    //--------------------------------------------------------------------------
    // allocate the result
    //--------------------------------------------------------------------------

    GB_CALLOC_MEMORY (Cp, GB_IMAX (2, Cnvec+1), sizeof (int64_t)) ;
    if (Cp == NULL)
    { 
        // out of memory
        return (GB_OUT_OF_MEMORY) ;
    }

    //--------------------------------------------------------------------------
    // count the entries in each vector of C
    //--------------------------------------------------------------------------

    #define GB_PHASE_1_OF_2
    #include "GB_add_template.c"

    //--------------------------------------------------------------------------
    // cumulative sum of Cp and fine tasks in TaskList
    //--------------------------------------------------------------------------

    GB_task_cumsum (Cp, Cnvec, Cnvec_nonempty, TaskList, ntasks, nthreads) ;

    //--------------------------------------------------------------------------
    // return the result
    //--------------------------------------------------------------------------

    (*Cp_handle) = Cp ;
    return (GrB_SUCCESS) ;
}

