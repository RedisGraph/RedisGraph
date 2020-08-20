//------------------------------------------------------------------------------
// GB_subref_phase2: C=A(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This function either frees Cp and Ch, or transplants then into C, as C->p
// and C->h.  Either way, the caller must not free them.

#include "GB_subref.h"
#include "GB_sort.h"

GrB_Info GB_subref_phase2   // C=A(I,J)
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    // from phase1:
    const int64_t *GB_RESTRICT Cp,         // vector pointers for C
    const int64_t Cnvec_nonempty,       // # of non-empty vectors in C
    // from phase0b:
    const GB_task_struct *GB_RESTRICT TaskList,    // array of structs
    const int ntasks,                           // # of tasks
    const int nthreads,                         // # of threads to use
    const bool post_sort,               // true if post-sort needed
    const int64_t *Mark,                // for I inverse buckets, size A->vlen
    const int64_t *Inext,               // for I inverse buckets, size nI
    const int64_t nduplicates,          // # of duplicates, if I inverted
    // from phase0:
    const int64_t *GB_RESTRICT Ch,
    const int64_t *GB_RESTRICT Ap_start,
    const int64_t *GB_RESTRICT Ap_end,
    const int64_t Cnvec,
    const bool need_qsort,
    const int Ikind,
    const int64_t nI,
    const int64_t Icolon [3],
    const int64_t nJ,
    // original input:
    const bool C_is_csc,        // format of output matrix C
    const GrB_Matrix A,
    const GrB_Index *I,
    const bool symbolic,
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Cp != NULL) ;
    ASSERT_MATRIX_OK (A, "A for subref phase2", GB0) ;

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    int64_t cnz = Cp [Cnvec] ;
    (*Chandle) = NULL ;

    bool C_is_hyper = (Ch != NULL) ;

    GrB_Type ctype = (symbolic) ? GrB_INT64 : A->type ;

    // allocate the result C (but do not allocate C->p or C->h)
    GrB_Info info ;
    GrB_Matrix C = NULL ;           // allocate a new header for C
    GB_CREATE (&C, ctype, nI, nJ, GB_Ap_null, C_is_csc,
        GB_SAME_HYPER_AS (C_is_hyper), A->hyper_ratio, Cnvec, cnz, true,
        Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_MEMORY (Cp, GB_IMAX (2, Cnvec+1), sizeof (int64_t)) ;
        GB_FREE_MEMORY (Ch, Cnvec, sizeof (int64_t)) ;
        return (info) ;
    }

    // add Cp as the vector pointers for C, from GB_subref_phase1
    C->p = (int64_t *) Cp ;

    // add Ch as the hypersparse list for C, from GB_subref_phase0
    if (C_is_hyper)
    { 
        // transplant Ch into C
        C->h = (int64_t *) Ch ;
        C->nvec = Cnvec ;
    }

    // now Cp and Ch have been transplanted into C, so they must not be freed.

    C->nvec_nonempty = Cnvec_nonempty ;
    C->magic = GB_MAGIC ;

    //--------------------------------------------------------------------------
    // phase2: C = A(I,J)
    //--------------------------------------------------------------------------

    #define GB_PHASE_2_OF_2
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
    // remove empty vectors from C, if hypersparse
    //--------------------------------------------------------------------------

    info = GB_hypermatrix_prune (C, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_MATRIX_FREE (&C) ;
        return (info) ;
    }

#if 0
    // see GB_hypermatrix_prune
    if (C_is_hyper && C->nvec_nonempty < Cnvec)
    {
        // create new Cp_new and Ch_new arrays, with no empty vectors
        int64_t *GB_RESTRICT Cp_new = NULL ;
        int64_t *GB_RESTRICT Ch_new = NULL ;
        int64_t nvec_new ;
        info = GB_hyper_prune (&Cp_new, &Ch_new, &nvec_new, C->p, C->h, Cnvec,
            Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory
            GB_MATRIX_FREE (&C) ;
            return (info) ;
        }
        // transplant the new hyperlist into C
        GB_FREE_MEMORY (C->p, Cnvec+1, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C->h, Cnvec,   sizeof (int64_t)) ;
        C->p = Cp_new ;
        C->h = Ch_new ;
        C->nvec = nvec_new ;
        C->plen = nvec_new ;
        ASSERT (C->nvec == C->nvec_nonempty) ;
    }
#endif

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // caller must not free Cp or Ch.   The matrix may have jumbled indices.
    // If it will be transposed in GB_accum_mask, but needs sorting, then the
    // sort is skipped since the transpose will handle the sort.
    ASSERT_MATRIX_OK_OR_JUMBLED (C, "C output for subref phase2", GB0) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

