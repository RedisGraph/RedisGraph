//------------------------------------------------------------------------------
// GB_subref_phase2: C=A(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// This function either frees Cp and Ch, or transplants then into C, as C->p
// and C->h.  Either way, the caller must not free them.

#include "GB_subref.h"
#include "GB_sort.h"

GrB_Info GB_subref_phase2   // C=A(I,J)
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    // from phase1:
    const int64_t *GB_RESTRICT *p_Cp,   // vector pointers for C
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
    const int64_t *GB_RESTRICT *p_Ch,
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

    const int64_t *GB_RESTRICT Ch = *p_Ch ;
    const int64_t *GB_RESTRICT Cp = *p_Cp ;
    ASSERT (Cp != NULL) ;
    ASSERT_MATRIX_OK (A, "A for subref phase2", GB0) ;
    ASSERT (!GB_IS_BITMAP (A)) ;    // GB_bitmap_subref is used instead

    //--------------------------------------------------------------------------
    // allocate the output matrix C
    //--------------------------------------------------------------------------

    int64_t cnz = Cp [Cnvec] ;
    (*Chandle) = NULL ;

    bool C_is_hyper = (Ch != NULL) ;

    GrB_Type ctype = (symbolic) ? GrB_INT64 : A->type ;

    // allocate the result C (but do not allocate C->p or C->h)
    GrB_Matrix C = NULL ;
    int sparsity = C_is_hyper ? GxB_HYPERSPARSE : GxB_SPARSE ;
    GrB_Info info = GB_new_bix (&C, // sparse or hyper, new header
        ctype, nI, nJ, GB_Ap_null, C_is_csc,
        sparsity, true, A->hyper_switch, Cnvec, cnz, true, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE (*p_Cp) ;
        GB_FREE (*p_Ch) ;
        return (info) ;
    }

    // add Cp as the vector pointers for C, from GB_subref_phase1
    C->p = (int64_t *) Cp ;
    (*p_Cp) = NULL ;

    // add Ch as the hypersparse list for C, from GB_subref_phase0
    if (C_is_hyper)
    { 
        // transplant Ch into C
        C->h = (int64_t *) Ch ;
        (*p_Ch) = NULL ;
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
        GB_Matrix_free (&C) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // caller must not free Cp or Ch
    ASSERT_MATRIX_OK (C, "C output for subref phase2", GB0) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

