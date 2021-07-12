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

// ctype is the type of matrix C, and currently it is always op->ztype,
// but this might change in the future.

// The pattern of C is the intersection of A and B, and also intersection with
// M if present and not complemented.

// TODO: if C is bitmap on input and C_sparsity is GxB_BITMAP, then C=A.*B,
// C<M>=A.*B and C<M>+=A.*B can all be done in-place.  Also, if C is bitmap
// but T<M>=A.*B is sparse (M sparse, with A and B bitmap), then it too can
// be done in place.

#include "GB_emult.h"
#include "GB_add.h"

#define GB_FREE_WORK                            \
{                                               \
    GB_FREE_WERK (&TaskList, TaskList_size) ;   \
    GB_FREE_WERK (&C_to_M, C_to_M_size) ;       \
    GB_FREE_WERK (&C_to_A, C_to_A_size) ;       \
    GB_FREE_WERK (&C_to_B, C_to_B_size) ;       \
}

#define GB_FREE_ALL             \
{                               \
    GB_FREE_WORK ;              \
    GB_phbix_free (C) ;       \
}

GrB_Info GB_emult           // C=A.*B, C<M>=A.*B, or C<!M>=A.*B
(
    GrB_Matrix C,           // output matrix, static header
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
    ASSERT (C != NULL && C->static_header) ;

    ASSERT_MATRIX_OK (A, "A for emult phased", GB0) ;
    ASSERT_MATRIX_OK (B, "B for emult phased", GB0) ;
    ASSERT_MATRIX_OK_OR_NULL (M, "M for emult phased", GB0) ;
    ASSERT_BINARYOP_OK (op, "op for emult phased", GB0) ;
    ASSERT (A->vdim == B->vdim && A->vlen == B->vlen) ;
    ASSERT (GB_IMPLIES (M != NULL, A->vdim == M->vdim && A->vlen == M->vlen)) ;

    //--------------------------------------------------------------------------
    // declare workspace
    //--------------------------------------------------------------------------

    GB_task_struct *TaskList = NULL ; size_t TaskList_size = 0 ;
    int64_t *restrict C_to_M = NULL ; size_t C_to_M_size = 0 ;
    int64_t *restrict C_to_A = NULL ; size_t C_to_A_size = 0 ;
    int64_t *restrict C_to_B = NULL ; size_t C_to_B_size = 0 ;

    //--------------------------------------------------------------------------
    // delete any lingering zombies and assemble any pending tuples
    //--------------------------------------------------------------------------

    // some cases can allow M, A, and/or B to be jumbled
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (M) ;
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (A) ;
    GB_MATRIX_WAIT_IF_PENDING_OR_ZOMBIES (B) ;

    //--------------------------------------------------------------------------
    // determine the sparsity of C and the method to use
    //--------------------------------------------------------------------------

    bool apply_mask ;           // if true, mask is applied during emult
    int ewise_method ;          // method to use
    int C_sparsity = GB_emult_sparsity (&apply_mask, &ewise_method,
        M, Mask_comp, A, B) ;

    //--------------------------------------------------------------------------
    // C<M or !M> = A.*B
    //--------------------------------------------------------------------------

    switch (ewise_method)
    {

        case GB_EMULT_METHOD_ADD :  // A and B both full (or as-if-full)

            //      ------------------------------------------
            //      C       =           A       .*      B
            //      ------------------------------------------
            //      full    .           full            full    (GB_add)
            //      ------------------------------------------
            //      C       <M> =       A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      full            full    (GB_add or 03)
            //      bitmap  bitmap      full            full    (GB_add or 07)
            //      bitmap  full        full            full    (GB_add or 07)
            //      ------------------------------------------
            //      C       <!M>=       A       .*      B
            //      ------------------------------------------
            //      bitmap  sparse      full            full    (GB_add or 06)
            //      bitmap  bitmap      full            full    (GB_add or 07)
            //      bitmap  full        full            full    (GB_add or 07)

            // A and B are both full (or as-if-full).  The mask M may be
            // anything.  GB_add computes the same thing in this case, so it is
            // used instead, to reduce the code needed for GB_emult.  GB_add
            // must be used for C=A.*B if all 3 matrices are full.  Otherwise,
            // GB_emult method can be used as well.

            return (GB_add (C, ctype, C_is_csc, M, Mask_struct,
                Mask_comp, mask_applied, A, B, op, Context)) ;

        case GB_EMULT_METHOD_02A :  // A sparse/hyper, B bitmap/full

            //      ------------------------------------------
            //      C       =           A       .*      B
            //      ------------------------------------------
            //      sparse  .           sparse          bitmap  (method: 02a)
            //      sparse  .           sparse          full    (method: 02a)
            //      ------------------------------------------
            //      C       <M> =       A       .*      B
            //      ------------------------------------------
            //      sparse  bitmap      sparse          bitmap  (method: 02a)
            //      sparse  bitmap      sparse          full    (method: 02a)
            //      sparse  full        sparse          bitmap  (method: 02a)
            //      sparse  full        sparse          full    (method: 02a)
            //      ------------------------------------------
            //      C       <!M>=       A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      sparse          bitmap  (02a: M later)
            //      sparse  sparse      sparse          full    (02a: M later)
            //      ------------------------------------------
            //      C       <!M> =       A       .*      B
            //      ------------------------------------------
            //      sparse  bitmap      sparse          bitmap  (method: 02a)
            //      sparse  bitmap      sparse          full    (method: 02a)
            //      sparse  full        sparse          bitmap  (method: 02a)
            //      sparse  full        sparse          full    (method: 02a)

            // A is sparse/hyper and B is bitmap/full.  M is either not
            // present, not applied (!M when sparse/hyper), or bitmap/full.
            // This method does not handle the case when M is sparse/hyper,
            // unless M is ignored and applied later.

            return (GB_emult_02 (C, ctype, C_is_csc,
                (apply_mask) ? M : NULL, Mask_struct, Mask_comp,
                A, B, op, false, Context)) ;

        case GB_EMULT_METHOD_02B :  // A bitmap/full, B sparse/hyper

            //      ------------------------------------------
            //      C       =           A       .*      B
            //      ------------------------------------------
            //      sparse  .           bitmap          sparse  (method: 02b)
            //      sparse  .           full            sparse  (method: 02b)
            //      ------------------------------------------
            //      C       <M> =       A       .*      B
            //      ------------------------------------------
            //      sparse  bitmap      bitmap          sparse  (method: 02b)
            //      sparse  bitmap      full            sparse  (method: 02b)
            //      sparse  full        bitmap          sparse  (method: 02b)
            //      sparse  full        full            sparse  (method: 02b)
            //      ------------------------------------------
            //      C       <!M>=       A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      bitmap          sparse  (02b: M later)
            //      sparse  sparse      full            sparse  (02b: M later)
            //      ------------------------------------------
            //      C       <!M> =      A       .*      B
            //      ------------------------------------------
            //      sparse  bitmap      bitmap          sparse  (method: 02b)
            //      sparse  bitmap      full            sparse  (method: 02b)
            //      sparse  full        bitmap          sparse  (method: 02b)
            //      sparse  full        full            sparse  (method: 02b)

            // A is bitmap/full and B is sparse/hyper, with flipxy true.
            // M is not present, not applied, or bitmap/full
            // Note that A and B are flipped.

            return (GB_emult_02 (C, ctype, C_is_csc,
                (apply_mask) ? M : NULL, Mask_struct, Mask_comp,
                B, A, op, true, Context)) ;

        case GB_EMULT_METHOD_01 : 

            //      ------------------------------------------
            //      C       =           A       .*      B
            //      ------------------------------------------
            //      sparse  .           sparse          sparse  (method: 01)
            //      ------------------------------------------
            //      C       <M> =       A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      sparse          sparse  (method: 01)
            //      sparse  bitmap      sparse          sparse  (method: 01)
            //      sparse  full        sparse          sparse  (method: 01)
            //      ------------------------------------------
            //      C       <!M>=       A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      sparse          sparse  (01: M later)
            //      sparse  bitmap      sparse          sparse  (method: 01)
            //      sparse  full        sparse          sparse  (method: 01)

            // TODO: break method 01 into different methods
            break ;

        case GB_EMULT_METHOD_05 :   // C is bitmap, M is not present

            //      ------------------------------------------
            //      C       =           A       .*      B
            //      ------------------------------------------
            //      bitmap  .           bitmap          bitmap  (method: 05)
            //      bitmap  .           bitmap          full    (method: 05)
            //      bitmap  .           full            bitmap  (method: 05)

        case GB_EMULT_METHOD_06 :   // C is bitmap, !M is sparse/hyper

            //      ------------------------------------------
            //      C       <!M>=       A       .*      B
            //      ------------------------------------------
            //      bitmap  sparse      bitmap          bitmap  (method: 06)
            //      bitmap  sparse      bitmap          full    (method: 06)
            //      bitmap  sparse      full            bitmap  (method: 06)
            //      bitmap  sparse      full            full    (GB_add or 06)

        case GB_EMULT_METHOD_07 :   // C is bitmap, M is bitmap/full

            //      ------------------------------------------
            //      C      <M> =        A       .*      B
            //      ------------------------------------------
            //      bitmap  bitmap      bitmap          bitmap  (method: 07)
            //      bitmap  bitmap      bitmap          full    (method: 07)
            //      bitmap  bitmap      full            bitmap  (method: 07)
            //      bitmap  bitmap      full            full    (GB_add or 07)
            //      bitmap  full        bitmap          bitmap  (method: 07)
            //      bitmap  full        bitmap          full    (method: 07)
            //      bitmap  full        full            bitmap  (method: 07)
            //      bitmap  full        full            full    (GB_add or 07)
            //      ------------------------------------------
            //      C      <!M> =       A       .*      B
            //      ------------------------------------------
            //      bitmap  bitmap      bitmap          bitmap  (method: 07)
            //      bitmap  bitmap      bitmap          full    (method: 07)
            //      bitmap  bitmap      full            bitmap  (method: 07)
            //      bitmap  bitmap      full            full    (GB_add or 07)
            //      bitmap  full        bitmap          bitmap  (method: 07)
            //      bitmap  full        bitmap          full    (method: 07)
            //      bitmap  full        full            bitmap  (method: 07)
            //      bitmap  full        full            full    (GB_add or 07)

            // For methods 05, 06, and 07, C is constructed as bitmap.
            // Both A and B are bitmap/full.  M is either not present,
            // complemented, or not complemented and bitmap/full.  The
            // case when M is not complemented and sparse/hyper is handled
            // by method 03, which constructs C as sparse/hyper (the same
            // structure as M), not bitmap.

            return (GB_bitmap_emult (C, ewise_method, ctype, C_is_csc,
                M, Mask_struct, Mask_comp, mask_applied, A, B,
                op, Context)) ;

        case GB_EMULT_METHOD_03 : 

            //      ------------------------------------------
            //      C       <M>=        A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      bitmap          bitmap  (method: 03)
            //      sparse  sparse      bitmap          full    (method: 03)
            //      sparse  sparse      full            bitmap  (method: 03)
            //      sparse  sparse      full            full    (GB_add or 03)

            return (GB_emult_03 (C, ctype, C_is_csc, M, Mask_struct,
                mask_applied, A, B, op, Context)) ;

        case GB_EMULT_METHOD_04A : break ; // punt

            //      ------------------------------------------
            //      C       <M>=        A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      sparse          bitmap  (method: 04a)
            //      sparse  sparse      sparse          full    (method: 04a)

            // TODO: this will use 04 (M,A,B, flipxy=false)

            // The method will compute the 2-way intersection of M and A,
            // using the same parallization as C=A.*B when both A and B are
            // both sparse.  It will then lookup B in O(1) time.
            // M and A must not be jumbled.

        case GB_EMULT_METHOD_04B : break ; // punt

            //      ------------------------------------------
            //      C       <M>=        A       .*      B
            //      ------------------------------------------
            //      sparse  sparse      bitmap          sparse  (method: 04b)
            //      sparse  sparse      full            sparse  (method: 04b)

            // TODO: this will use 04 (M,B,A, flipxy=true)
            // M and B must not be jumbled.

        default:;
    }

    //--------------------------------------------------------------------------
    // method 01 (and for now, 04a and 04b)
    //--------------------------------------------------------------------------

    ASSERT (C_sparsity == GxB_SPARSE || C_sparsity == GxB_HYPERSPARSE) ;

    GB_MATRIX_WAIT (M) ;
    GB_MATRIX_WAIT (A) ;
    GB_MATRIX_WAIT (B) ;

    GBURBLE ("emult:(%s<%s>=%s.*%s) ",
        GB_sparsity_char (C_sparsity),
        GB_sparsity_char_matrix (M),
        GB_sparsity_char_matrix (A),
        GB_sparsity_char_matrix (B)) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    int64_t Cnvec, Cnvec_nonempty ;
    int64_t *Cp = NULL ; size_t Cp_size = 0 ;
    const int64_t *Ch = NULL ; size_t Ch_size = 0 ;
    int C_ntasks = 0, C_nthreads ;

    //--------------------------------------------------------------------------
    // phase0: finalize the sparsity C and find the vectors in C
    //--------------------------------------------------------------------------

    GB_OK (GB_emult_01_phase0 (
        // computed by phase0:
        &Cnvec, &Ch, &Ch_size, &C_to_M, &C_to_M_size, &C_to_A, &C_to_A_size,
        &C_to_B, &C_to_B_size,
        // input/output to phase0:
        &C_sparsity,
        // original input:
        (apply_mask) ? M : NULL, A, B, Context)) ;

    // C is still sparse or hypersparse, not bitmap or full
    ASSERT (C_sparsity == GxB_SPARSE || C_sparsity == GxB_HYPERSPARSE) ;

    //--------------------------------------------------------------------------
    // phase1: split C into tasks, and count entries in each vector of C
    //--------------------------------------------------------------------------

    // phase1a: split C into tasks
    GB_OK (GB_ewise_slice (
        // computed by phase1a:
        &TaskList, &TaskList_size, &C_ntasks, &C_nthreads,
        // computed by phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B, false,
        // original input:
        (apply_mask) ? M : NULL, A, B, Context)) ;

    // count the number of entries in each vector of C
    GB_OK (GB_emult_01_phase1 (
        // computed by phase1:
        &Cp, &Cp_size, &Cnvec_nonempty,
        // from phase1a:
        TaskList, C_ntasks, C_nthreads,
        // from phase0:
        Cnvec, Ch, C_to_M, C_to_A, C_to_B,
        // original input:
        (apply_mask) ? M : NULL, Mask_struct, Mask_comp, A, B, Context)) ;

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of C
    //--------------------------------------------------------------------------

    // Cp is either freed by phase2, or transplanted into C.
    // Either way, it is not freed here.

    GB_OK (GB_emult_01_phase2 (
        // computed or used by phase2:
        C, ctype, C_is_csc, op,
        // from phase1:
        &Cp, Cp_size, Cnvec_nonempty,
        // from phase1a:
        TaskList, C_ntasks, C_nthreads,
        // from phase0:
        Cnvec, Ch, Ch_size, C_to_M, C_to_A, C_to_B, C_sparsity,
        // from GB_emult_sparsity:
        ewise_method,
        // original input:
        (apply_mask) ? M : NULL, Mask_struct, Mask_comp, A, B, Context)) ;

    //--------------------------------------------------------------------------
    // free workspace and return result
    //--------------------------------------------------------------------------

    GB_FREE_WORK ;
    ASSERT_MATRIX_OK (C, "C output for emult phased", GB0) ;
    (*mask_applied) = apply_mask ;
    return (GrB_SUCCESS) ;
}

