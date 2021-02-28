//------------------------------------------------------------------------------
// GB_emult_phase1: find # of entries in C=A.*B or C<M>=A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_emult_phase1 counts the number of entries in each vector of C, for
// C=A.*B or C<M>=A.*B and then does a cumulative sum to find Cp.
// GB_emult_phase1 is preceded by GB_emult_phase0, which finds the non-empty
// vectors of C.  This phase is done entirely in parallel.

// C, M, A, and B can be standard sparse or hypersparse, as determined by
// GB_emult_phase0.  If present, the mask M is not complemented.

// Cp is either freed by GB_emult_phase2, or transplanted into C.

#include "GB_emult.h"

GrB_Info GB_emult_phase1                // count nnz in each C(:,j)
(
    int64_t *GB_RESTRICT *Cp_handle,       // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,  // array of structs
    const int ntasks,                   // # of tasks
    const int nthreads,                 // # of threads to use
    // analysis from phase0:
    const int64_t Cnvec,
    const int64_t *GB_RESTRICT Ch,         // Ch is NULL, or shallow pointer
    const int64_t *GB_RESTRICT C_to_M,
    const int64_t *GB_RESTRICT C_to_A,
    const int64_t *GB_RESTRICT C_to_B,
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
    ASSERT_MATRIX_OK (A, "A for emult phase1", GB0) ;
    ASSERT_MATRIX_OK (B, "B for emult phase1", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for emult phase1", GB0) ;
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
    #include "GB_emult_template.c"

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

