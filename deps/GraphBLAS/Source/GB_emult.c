//------------------------------------------------------------------------------
// GB_emult: C = A.*B or C<M>=A.*B
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// GB_emult, does C=A.*B or C<M>=A.*B, using the given operator element-wise on
// the matrices A and B.  The result is typecasted as needed.  The pattern of C
// is the intersection of the pattern of A and B, intersection with the mask M,
// if present and not complemented.  The complemented mask is not handled here,
// but in GB_mask.

// Let the op be z=f(x,y) where x, y, and z have type xtype, ytype, and ztype.
// If both A(i,j) and B(i,j) are present, then:

//      C(i,j) = (ctype) op ((xtype) A(i,j), (ytype) B(i,j))

// If just A(i,j) is present but not B(i,j), or
// if just B(i,j) is present but not A(i,j), then C(i,j) is not present.

// ctype is the type of matrix C.  The pattern of C is the intersection of A
// and B, and also intersection with M if present.

#include "GB_emult.h"

GrB_Info GB_emult           // C=A.*B or C<M>=A.*B
(
    GrB_Matrix *Chandle,    // output matrix (unallocated on input)
    const GrB_Type ctype,   // type of output matrix C
    const bool C_is_csc,    // format of output matrix C
    const GrB_Matrix M,     // optional mask, unused if NULL.  Not complemented
    const bool Mask_struct, // if true, use the only structure of M
    const GrB_Matrix A,     // input A matrix
    const GrB_Matrix B,     // input B matrix
    const GrB_BinaryOp op,  // op to perform C = op (A,B)
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GBBURBLE ((M == NULL) ? "emult " : "masked_emult ") ;

    ASSERT (Chandle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for emult phased", GB0) ;
    ASSERT_MATRIX_OK (B, "B for emult phased", GB0) ;
    ASSERT_BINARYOP_OK_OR_NULL (op, "op for emult phased", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for emult phased", GB0) ;
    ASSERT (!GB_PENDING (A)) ; ASSERT (!GB_ZOMBIES (A)) ;
    ASSERT (!GB_PENDING (B)) ; ASSERT (!GB_ZOMBIES (B)) ;
    ASSERT (A->vdim == B->vdim && A->vlen == B->vlen) ;
    if (M != NULL)
    { 
        ASSERT (!GB_PENDING (M)) ; ASSERT (!GB_ZOMBIES (M)) ;
        ASSERT (A->vdim == M->vdim && A->vlen == M->vlen) ;
    }

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Matrix C = NULL ;
    (*Chandle) = NULL ;
    int64_t Cnvec, Cnvec_nonempty ;
    int64_t *GB_RESTRICT Cp = NULL ;
    const int64_t *GB_RESTRICT Ch = NULL ;
    int64_t *GB_RESTRICT C_to_M = NULL ;
    int64_t *GB_RESTRICT C_to_A = NULL ;
    int64_t *GB_RESTRICT C_to_B = NULL ;
    int ntasks, max_ntasks, nthreads ;
    GB_task_struct *TaskList = NULL ;

    //--------------------------------------------------------------------------
    // phase0: determine the vectors in C(:,j)
    //--------------------------------------------------------------------------

    GrB_Info info = GB_emult_phase0 (
        // computed by phase0:
        &Cnvec, &Ch, &C_to_M, &C_to_A, &C_to_B,
        // original input:
        M, A, B, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase0b: split C into tasks for phase1 and phase2
    //--------------------------------------------------------------------------

    info = GB_ewise_slice (
        // computed by phase0b:
        &TaskList, &max_ntasks, &ntasks, &nthreads,
        // computed by phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B, false,
        // original input:
        M, A, B, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; free everything allocated by GB_emult_phase0
        GB_FREE_MEMORY (C_to_M, Cnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C_to_A, Cnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C_to_B, Cnvec, sizeof (int64_t)) ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase1: count the number of entries in each vector of C
    //--------------------------------------------------------------------------

    info = GB_emult_phase1 (
        // computed by phase1:
        &Cp, &Cnvec_nonempty,
        // from phase0b:
        TaskList, ntasks, nthreads,
        // from phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B,
        // original input:
        M, Mask_struct, A, B, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory; free everything allocated by phase 0
        GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;
        GB_FREE_MEMORY (C_to_M, Cnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C_to_A, Cnvec, sizeof (int64_t)) ;
        GB_FREE_MEMORY (C_to_B, Cnvec, sizeof (int64_t)) ;
        return (info) ;
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
        // from phase0b:
        TaskList, ntasks, nthreads,
        // from phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B,
        // original input:
        M, Mask_struct, A, B, Context) ;

    // free workspace
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;
    GB_FREE_MEMORY (C_to_M, Cnvec, sizeof (int64_t)) ;
    GB_FREE_MEMORY (C_to_A, Cnvec, sizeof (int64_t)) ;
    GB_FREE_MEMORY (C_to_B, Cnvec, sizeof (int64_t)) ;

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
    return (GrB_SUCCESS) ;
}

