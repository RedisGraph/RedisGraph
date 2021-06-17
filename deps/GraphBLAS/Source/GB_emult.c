//------------------------------------------------------------------------------
// GB_emult: C = A.*B, C<M>=A.*B, or C<!M>=A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// GB_emult, does C=A.*B, C<M>=A.*B, C<!M>=A.*B, using the given operator
// element-wise on the matrices A and B.  The result is typecasted as needed.
// The pattern of C is the intersection of the pattern of A and B, intersection
// with the mask M or !M, if present.

// Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
// If both A(i,j) and B(i,j) are present, then:

//      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))

// If just A(i,j) is present but not B(i,j), or
// if just B(i,j) is present but not A(i,j), then C(i,j) is not present.

// ctype is the type of matrix C.  The pattern of C is the intersection of A
// and B, and also intersection with M if present.

#include "GB_emult.h"
#include "GB_add.h"

#define GB_FREE_ALL ;

GrB_Info GB_emult           // C=A.*B, C<M>=A.*B, or C<!M>=A.*B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask, unused if NULL
    const bool Mask_struct, // if true, use the only structure of M
    const bool Mask_comp,   // if true, use !M
    bool *mask_applied,     // if true, the mask was applied
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;

    ASSERT (Chandle != NULL) ;
    GrB_Matrix C = NULL ;
    (*Chandle) = NULL ;

    ASSERT_MATRIX_OK (A, "A for emult phased", GB0) ;
    ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_JUMBLED (A)) ;
    ASSERT (!GB_PENDING (A)) ;

    ASSERT_MATRIX_OK (B, "B for emult phased", GB0) ;
    ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (!GB_JUMBLED (B)) ;
    ASSERT (!GB_PENDING (B)) ;

    ASSERT_MATRIX_OK_OR_NULL (M, "M for emult phased", GB0) ;
    ASSERT (!GB_ZOMBIES (M)) ;
    ASSERT (!GB_JUMBLED (M)) ;
    ASSERT (!GB_PENDING (M)) ;

    ASSERT_BINARYOP_OK_OR_NULL (op, "op for emult phased", GB0) ;
    ASSERT (A->vdim == B->vdim && A->vlen == B->vlen) ;
    ASSERT (GB_IMPLIES (M != NULL, A->vdim == M->vdim && A->vlen == M->vlen)) ;

    //--------------------------------------------------------------------------
    // determine the sparsity of C
    //--------------------------------------------------------------------------

    bool apply_mask, use_add_instead ;
    int C_sparsity = GB_emult_sparsity (&apply_mask, &use_add_instead,
        M, Mask_comp, A, B) ;

    //--------------------------------------------------------------------------
    // use GB_add instead, as determined by GB_emult_sparsity
    //--------------------------------------------------------------------------

    if (use_add_instead)
    { 
        // A and B are both full.  The mask M may be present or not, and may be
        // complemented or not.  GB_add computes the same thing in this case,
        // so use it instead, to reduce the code needed for GB_emult.
        return (GB_add (Chandle, ctype, C_is_csc, M, Mask_struct, Mask_comp,
            mask_applied, A, B, op, Context)) ;
    }

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    int64_t Cnvec, Cnvec_nonempty ;
    int64_t *GB_RESTRICT Cp = NULL ;
    const int64_t *GB_RESTRICT Ch = NULL ;  // shallow; must not be freed
    int64_t *GB_RESTRICT C_to_M = NULL ;
    int64_t *GB_RESTRICT C_to_A = NULL ;
    int64_t *GB_RESTRICT C_to_B = NULL ;
    int C_ntasks = 0, TaskList_size = 0, C_nthreads ;
    GB_task_struct *TaskList = NULL ;

    //--------------------------------------------------------------------------
    // phase0: finalize the sparsity C and find the vectors in C
    //--------------------------------------------------------------------------

    info = GB_emult_phase0 (
        // computed by phase0:
        &Cnvec, &Ch, &C_to_M, &C_to_A, &C_to_B,
        // input/output to phase0:
        &C_sparsity,
        // original input:
        (apply_mask) ? M : NULL, A, B, Context) ;
    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    GBURBLE ("emult:(%s<%s>=%s.*%s) ",
        GB_sparsity_char (C_sparsity),
        GB_sparsity_char_matrix (M),
        GB_sparsity_char_matrix (A),
        GB_sparsity_char_matrix (B)) ;

    //--------------------------------------------------------------------------
    // phase1: split C into tasks, and count entries in each vector of C
    //--------------------------------------------------------------------------

    if (C_sparsity == GxB_SPARSE || C_sparsity == GxB_HYPERSPARSE)
    {

        //----------------------------------------------------------------------
        // C is sparse or hypersparse: slice and analyze the C matrix
        //----------------------------------------------------------------------

        // phase1a: split C into tasks
        info = GB_ewise_slice (
            // computed by phase1a:
            &TaskList, &TaskList_size, &C_ntasks, &C_nthreads,
            // computed by phase0:
            Cnvec, Ch, C_to_M, C_to_A, C_to_B, false,
            // original input:
            (apply_mask) ? M : NULL, A, B, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory; free everything allocated by GB_emult_phase0
            GB_FREE (C_to_M) ;
            GB_FREE (C_to_A) ;
            GB_FREE (C_to_B) ;
            return (info) ;
        }

        // count the number of entries in each vector of C
        info = GB_emult_phase1 (
            // computed by phase1:
            &Cp, &Cnvec_nonempty,
            // from phase1a:
            TaskList, C_ntasks, C_nthreads,
            // from phase0:
            Cnvec, Ch, C_to_M, C_to_A, C_to_B,
            // original input:
            (apply_mask) ? M : NULL, Mask_struct, Mask_comp, A, B, Context) ;
        if (info != GrB_SUCCESS)
        { 
            // out of memory; free everything allocated by phase 0
            GB_FREE (TaskList) ;
            GB_FREE (C_to_M) ;
            GB_FREE (C_to_A) ;
            GB_FREE (C_to_B) ;
            return (info) ;
        }

    }
    else
    { 

        //----------------------------------------------------------------------
        // C is bitmap or full: only determine how many threads to use
        //----------------------------------------------------------------------

        GB_GET_NTHREADS_MAX (nthreads_max, chunk, Context) ;
        C_nthreads = GB_nthreads (A->vlen * A->vdim, chunk, nthreads_max) ;
    }

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of C
    //--------------------------------------------------------------------------

    // Cp is either freed by phase2, or transplanted into C.
    // Either way, it is not freed here.

    info = GB_emult_phase2 (
        // computed or used by phase2:
        &C, ctype, C_is_csc, op,
        // from phase1:
        Cp, Cnvec_nonempty,
        // from phase1a:
        TaskList, C_ntasks, C_nthreads,
        // from phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B, C_sparsity,
        // original input:
        (apply_mask) ? M : NULL, Mask_struct, Mask_comp, A, B, Context) ;

    // free workspace
    GB_FREE (TaskList) ;
    GB_FREE (C_to_M) ;
    GB_FREE (C_to_A) ;
    GB_FREE (C_to_B) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; note that Cp is already freed, and Ch is shallow
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    ASSERT_MATRIX_OK (C, "C output for emult phased", GB0) ;
    (*Chandle) = C ;
    ASSERT (!GB_ZOMBIES (C)) ;
    ASSERT (!GB_JUMBLED (C)) ;
    ASSERT (!GB_PENDING (C)) ;
    return (GrB_SUCCESS) ;
}

