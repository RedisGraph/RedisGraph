//------------------------------------------------------------------------------
// GB_subref_phase1: find # of entries in C=A(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_subref_phase1 counts the number of entries in each vector of C, for
// C=A(I,J) and then does a cumulative sum to find Cp.

// Cp is either freed by phase2, or transplanted into C.

#include "GB_subref.h"

GrB_Info GB_subref_phase1               // count nnz in each C(:,j)
(
    int64_t *GB_RESTRICT *Cp_handle,       // output of size Cnvec+1
    int64_t *Cnvec_nonempty,            // # of non-empty vectors in C
    // tasks from phase0b:
    GB_task_struct *GB_RESTRICT TaskList,  // array of structs
    const int ntasks,                   // # of tasks
    const int nthreads,                 // # of threads to use
    const int64_t *Mark,                // for I inverse buckets, size A->vlen
    const int64_t *Inext,               // for I inverse buckets, size nI
    const int64_t nduplicates,          // # of duplicates, if I inverted
    // analysis from phase0:
    const int64_t *GB_RESTRICT Ap_start,
    const int64_t *GB_RESTRICT Ap_end,
    const int64_t Cnvec,
    const bool need_qsort,
    const int Ikind,
    const int64_t nI,
    const int64_t Icolon [3],
    // original input:
    const GrB_Matrix A,
    const GrB_Index *I,         // index list for C = A(I,J), or GrB_ALL, etc.
    const bool symbolic,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cp_handle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for subref phase1", GB0) ;

    //--------------------------------------------------------------------------
    // allocate the result
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Cp = NULL ;
    (*Cp_handle) = NULL ;

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
    if (symbolic)
    {
        #define GB_SYMBOLIC
        #include "GB_subref_template.c"
        #undef  GB_SYMBOLIC
    }
    else
    {
        #define GB_NUMERIC
        #include "GB_subref_template.c"
        #undef  GB_NUMERIC
    }

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

