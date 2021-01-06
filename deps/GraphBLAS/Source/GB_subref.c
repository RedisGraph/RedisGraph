//------------------------------------------------------------------------------
// GB_subref: C = A(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// C=A(I,J), either symbolic or numeric.  In a symbolic extraction, Cx [p] is
// not the value of A(i,j), but its position in Ai,Ax.  That is, pA = Cx [p]
// means that the entry at position p in C is the same as the entry in A at
// position pA.  In this case, Cx has a type of int64_t.

// Numeric extraction:

//      Sparse submatrix reference, C = A(I,J), extracting the values.  This is
//      an internal function called by GB_extract with symbolic==false, which
//      does the work of the user-callable GrB_*_extract methods.  It is also
//      called by GB_assign to extract the submask.  No pending tuples or
//      zombies appear in A.

// Symbolic extraction:

//      Sparse submatrix reference, C = A(I,J), extracting the pattern, not the
//      values.  For the symbolic case, this function is called only by
//      GB_subassign_symbolic.  Symbolic extraction creates a matrix C with the
//      same pattern (C->p and C->i) as numeric extraction, but with different
//      values, C->x.  For numeric extracion if C(inew,jnew) = A(i,j), the
//      value of A(i,j) is copied into C(i,j).  For symbolic extraction, its
//      *pointer* is copied into C(i,j).  Suppose an entry A(i,j) is held in Ai
//      [pa] and Ax [pa], and it appears in the output matrix C in Ci [pc] and
//      Cx [pc].  Then the two methods differ as follows:

//          this is the same:

//          i = Ai [pa] ;           // index i of entry A(i,j)

//          aij = Ax [pa] ;         // value of the entry A(i,j)

//          Ci [pc] = inew ;        // index inew of C(inew,jnew)

//          this is different:

//          Cx [pc] = aij ;         // for numeric extraction

//          Cx [pc] = pa ;          // for symbolic extraction

//      This function is called with symbolic==true by only by
//      GB_subassign_symbolic, which uses it to extract the pattern of C(I,J),
//      for the submatrix assignment C(I,J)=A.  In this case, this function
//      needs to deal with zombie entries.  GB_subassign_symbolic uses this
//      function on its C matrix, which is called A here because it is not
//      modified here.

//      Reading a zombie entry:  A zombie entry A(i,j) has been marked by
//      flipping its index.  The value of a zombie is not important, just its
//      presence in the pattern.  All zombies have been flipped (i < 0), and
//      all regular entries are not flipped (i >= 0).  Zombies are entries that
//      have been marked for deletion but have not been removed from the matrix
//      yet, since it's more efficient to delete zombies all at once rather
//      than one at a time.

//      The symbolic case is zombie-agnostic, in the sense that it does not
//      delete them.  It treats them like regular entries.  However, their
//      normal index must be used, not their flipped indices.  The output
//      matrix C contains all unflipped indices, and its references to zombies
//      and regular entries are identical.  Zombies in A are dealt with later.
//      They cannot be detected in the output C matrix, but they can be
//      detected in A.  Since pa = Cx [pc] holds the position of the entry in
//      A, the entry is a zombie if Ai [pa] has been flipped.

#define GB_FREE_WORK        \
{                           \
    GB_FREE (TaskList) ;    \
    GB_FREE (Ap_start) ;    \
    GB_FREE (Ap_end) ;      \
    GB_FREE (Mark) ;        \
    GB_FREE (Inext) ;       \
}

#define GB_FREE_ALL         \
{                           \
    GB_FREE (Cp) ;          \
    GB_FREE (Ch) ;          \
    GB_FREE_WORK ;          \
}

#include "GB_subref.h"

GB_PUBLIC   // accessed by the MATLAB tests in GraphBLAS/Test only
GrB_Info GB_subref              // C = A(I,J): either symbolic or numeric
(
    // output
    GrB_Matrix *Chandle,
    // input, not modified
    const bool C_is_csc,        // requested format of C
    const GrB_Matrix A,
    const GrB_Index *I,         // index list for C = A(I,J), or GrB_ALL, etc.
    const int64_t ni,           // length of I, or special
    const GrB_Index *J,         // index list for C = A(I,J), or GrB_ALL, etc.
    const int64_t nj,           // length of J, or special
    const bool symbolic,        // if true, construct C as symbolic
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Info info ;
    ASSERT (Chandle != NULL) ;
    ASSERT_MATRIX_OK (A, "A for C=A(I,J) subref", GB0) ;
    ASSERT (GB_ZOMBIES_OK (A)) ;
    ASSERT (GB_JUMBLED_OK (A)) ;    // A is sorted, below, if jumbled on input
    ASSERT (GB_PENDING_OK (A)) ;

    //--------------------------------------------------------------------------
    // handle bitmap and full cases
    //--------------------------------------------------------------------------

    if (GB_IS_BITMAP (A) || GB_IS_FULL (A))
    { 
        // C is constructed with same sparsity as A (bitmap or full)
        return (GB_bitmap_subref (Chandle, C_is_csc, A, I, ni, J, nj, symbolic,
            Context)) ;
    }

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    int64_t *GB_RESTRICT Cp = NULL ;
    int64_t *GB_RESTRICT Ch = NULL ;
    int64_t *GB_RESTRICT Ap_start = NULL ;
    int64_t *GB_RESTRICT Ap_end = NULL ;
    int64_t *GB_RESTRICT Mark = NULL ;
    int64_t *GB_RESTRICT Inext = NULL ;
    GB_task_struct *TaskList = NULL ;
    GrB_Matrix C = NULL ;

    int64_t Cnvec = 0, nI = 0, nJ, Icolon [3], Cnvec_nonempty, ndupl ;
    bool post_sort, need_qsort ;
    int Ikind, ntasks, max_ntasks = 0, nthreads ;

    //--------------------------------------------------------------------------
    // ensure A is unjumbled
    //--------------------------------------------------------------------------

    // ensure input matrix is not jumbled.  Zombies are OK.
    GB_MATRIX_WAIT_IF_JUMBLED (A) ;

    //--------------------------------------------------------------------------
    // phase0: find vectors for C=A(I,J), and I,J properties
    //--------------------------------------------------------------------------

    GB_OK (GB_subref_phase0 (
        // computed by phase0:
        &Ch, &Ap_start, &Ap_end, &Cnvec, &need_qsort, &Ikind, &nI, Icolon, &nJ,
        // original input:
        A, I, ni, J, nj, Context)) ;

    //--------------------------------------------------------------------------
    // phase0b: split C=A(I,J) into tasks for phase1 and phase2
    //--------------------------------------------------------------------------

    // This phase also inverts I if needed.

    GB_OK (GB_subref_slice (
        // computed by phase0b:
        &TaskList, &max_ntasks, &ntasks, &nthreads, &post_sort,
        &Mark, &Inext, &ndupl,
        // computed by phase0:
        Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon,
        // original input:
        A->vlen, GB_NNZ (A), I, Context)) ;

    //--------------------------------------------------------------------------
    // phase1: count the number of entries in each vector of C
    //--------------------------------------------------------------------------

    GB_OK (GB_subref_phase1 (
        // computed by phase1:
        &Cp, &Cnvec_nonempty,
        // computed by phase0b:
        TaskList, ntasks, nthreads, Mark, Inext, ndupl,
        // computed by phase0:
        Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon,
        // original input:
        A, I, symbolic, Context)) ;

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of C
    //--------------------------------------------------------------------------

    GB_OK (GB_subref_phase2 (
        // computed by phase2:
        &C,
        // from phase1:
        &Cp, Cnvec_nonempty,
        // from phase0b:
        TaskList, ntasks, nthreads, post_sort, Mark, Inext, ndupl,
        // from phase0:
        &Ch, Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon, nJ,
        // original input:
        C_is_csc, A, I, symbolic, Context)) ;

    // Cp and Ch have been imported into C->p and C->h, or freed if phase2
    // fails.  Either way, Cp and Ch are set to NULL so that they cannot be
    // freed here (except by freeing C itself).

    // free workspace
    GB_FREE_WORK ;

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    // C can be returned jumbled, even if A is not jumbled
    ASSERT_MATRIX_OK (C, "C output for C=A(I,J)", GB0) ;
    ASSERT (GB_ZOMBIES_OK (C)) ;
    ASSERT (GB_JUMBLED_OK (C)) ;
    ASSERT (GB_IS_SPARSE (A) || GB_IS_HYPERSPARSE (A)) ;
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

