//------------------------------------------------------------------------------
// GB_subref: C = A(I,J)
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

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
//      GB_subassigner.  Symbolic extraction creates a matrix C with the same
//      pattern (C->p and C->i) as numeric extraction, but with different
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

//      This function is called with symbolic==true by only by GB_subassigner,
//      which uses it to extract the pattern of C(I,J), for the submatrix
//      assignment C(I,J)=A.  In this case, this function needs to deal with
//      zombie entries.  GB_subassigner uses this function on its C matrix,
//      which is called A here because it is not modified here.

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

#define GB_FREE_WORK                                                        \
{                                                                           \
    GB_FREE_MEMORY (TaskList, max_ntasks+1, sizeof (GB_task_struct)) ;      \
    GB_FREE_MEMORY (Ap_start, Cnvec, sizeof (int64_t)) ;                    \
    GB_FREE_MEMORY (Ap_end,   Cnvec, sizeof (int64_t)) ;                    \
    GB_FREE_MEMORY (Mark,     A->vlen, sizeof (int64_t)) ;                  \
    GB_FREE_MEMORY (Inext,    nI, sizeof (int64_t)) ;                       \
}

#include "GB_subref.h"

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
    const bool symbolic,        // if true, construct Cx as symbolic
    const bool must_sort,       // if true, must return C sorted
    GB_Context Context
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    ASSERT (Chandle != NULL) ;
    ASSERT_OK (GB_check (A, "A for C=A(I,J) subref", GB0)) ;

    //--------------------------------------------------------------------------
    // phase0: find vectors for C=A(I,J), and I,J properties
    //--------------------------------------------------------------------------

    int64_t *restrict Cp = NULL ;
    int64_t *restrict Ch = NULL ;
    int64_t *restrict Ap_start = NULL ;
    int64_t *restrict Ap_end = NULL ;
    int64_t *restrict Mark = NULL ;
    int64_t *restrict Inext = NULL ;
    GB_task_struct *TaskList = NULL ;
    GrB_Matrix C = NULL ;

    int64_t Cnvec = 0, nI = 0, nJ, Icolon [3], Cnvec_nonempty, ndupl ;
    bool post_sort, need_qsort ;
    int Ikind, ntasks, max_ntasks = 0, nthreads ;

    GrB_Info info = GB_subref_phase0 (
        // computed by phase0:
        &Ch, &Ap_start, &Ap_end, &Cnvec, &need_qsort, &Ikind, &nI, Icolon, &nJ,
        // original input:
        A, I, ni, J, nj, must_sort, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // I,J invalid, or out of memory
        GB_FREE_WORK ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase0b: split C=A(I,J) into tasks for phase1 and phase2
    //--------------------------------------------------------------------------

    // This phase also inverts I if needed.

    info = GB_subref_slice (
        // computed by phase0b:
        &TaskList, &max_ntasks, &ntasks, &nthreads, &post_sort,
        &Mark, &Inext, &ndupl,
        // computed by phase0:
        Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon,
        // original input:
        A->vlen, GB_NNZ (A), I, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_MEMORY (Ch, Cnvec, sizeof (int64_t)) ;
        GB_FREE_WORK ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase1: count the number of entries in each vector of C
    //--------------------------------------------------------------------------

    info = GB_subref_phase1 (
        // computed by phase1:
        &Cp, &Cnvec_nonempty,
        // computed by phase0b:
        TaskList, ntasks, nthreads, Mark, Inext, ndupl,
        // computed by phase0:
        Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon,
        // original input:
        A, I, symbolic, Context) ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        GB_FREE_MEMORY (Ch, Cnvec, sizeof (int64_t)) ;
        GB_FREE_WORK ;
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // phase2: compute the entries (indices and values) in each vector of C
    //--------------------------------------------------------------------------

    info = GB_subref_phase2 (
        // computed by phase2:
        &C,
        // from phase1:
        Cp, Cnvec_nonempty,
        // from phase0b:
        TaskList, ntasks, nthreads, post_sort, Mark, Inext, ndupl,
        // from phase0:
        Ch, Ap_start, Ap_end, Cnvec, need_qsort, Ikind, nI, Icolon, nJ,
        // original input:
        C_is_csc, A, I, symbolic, Context) ;

    // free workspace
    GB_FREE_WORK ;

    if (info != GrB_SUCCESS)
    { 
        // out of memory
        return (info) ;
    }

    //--------------------------------------------------------------------------
    // return result
    //--------------------------------------------------------------------------

    if (must_sort)
    {
        ASSERT_OK (GB_check (C, "sorted C output for C=A(I,J)", GB0)) ;
    }
    else
    {
        // The matrix may have jumbled indices.  If it will be transposed in
        // GB_accum_mask, but needs sorting, then the sort is skipped since the
        // transpose will handle the sort.
        ASSERT_OK_OR_JUMBLED (GB_check (C, "C output for C=A(I,J)", GB0)) ;
    }
    (*Chandle) = C ;
    return (GrB_SUCCESS) ;
}

