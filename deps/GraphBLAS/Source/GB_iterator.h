//------------------------------------------------------------------------------
// GB_iterator.h: definitions for the GrB_Matrix iterator
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

#ifndef GB_ITERATOR_H
#define GB_ITERATOR_H
#include "GB.h"

// GBI_single_iterator: controls the iteration over the vectors of a single
// matrix, which can be in any format (standard, hypersparse, slice, or
// hyperslice).  It is easily parallelizable if the iterations are independent,
// or for reduction-style loops via the appropriate #pragmas.

//------------------------------------------------------------------------------
// GBI_single_iterator: iterate over the vectors of a matrix
//------------------------------------------------------------------------------

// The Iter->* content of a GBI_single_iterator is accessed only in this file.
// All typedefs, functions, and macros that operate on the
// SuiteSparse:GraphBLAS iterator have names that start with the GBI prefix.
// For both kinds of iterators, the A->h and A->p components of the matrices
// may not change during the iteration.

// The single-matrix iterator, GBI_for_each_vector (A) can handle any of the
// four cases: standard, hypersparse, slice, or hyperslice.  The comments below
// assume A is in CSC format.

#ifdef for_comments_only    // only so vim will add color to the code below:

    // The GBI_for_each_vector (A) macro, which uses the GBI_single_iterator,
    // the two functions GBI1_init and GBI1_start, and the macro
    // GBI_jth_iteration can do any one of the 4 following for loops, depending
    // on whether A is standard, hypersparse, a slice, or a hyperslice.

    // A->vdim: the vector dimension of A (ncols(A))
    // A->nvec: # of vectors that appear in A.  For the hypersparse case,
    //          these are the number of column indices in Ah [0..nvec-1], since
    //          A is CSC.  For all cases, Ap [0...nvec] are the pointers.

    //--------------------
    // (1) standard     // A->is_hyper == false, A->is_slice == false
                        // A->nvec == A->vdim, A->hfirst == 0

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = k ;
            // operate on column A(:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
            }
        }

    //--------------------
    // (2) hypersparse  // A->is_hyper == true, A->is_slice == false
                        // A->nvec <= A->dim, A->hfirst == 0 (ignored)

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->h [k]
            // operate on column A(:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
            }
        }

    //--------------------
    // (3) slice, of another standard matrix S.
                        // A->i == S->i, A->x == S->x
                        // A->p = S->p + A->hfirst, A->h is NULL
                        // A->nvec <= A->vdim == S->vdim

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->hfirst + k ;
            // operate on column A(:,j), which is also S (:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p]
                // This is identical to S(i,j)
            }
        }

    //--------------------
    // (4) hyperslice, of another hypersparse matrix S
                        // A->i == S->i, A->x == S->x, A->p = S->p + kfirst,
                        // A->h == S->h + kfirst where A(:,0) is the same
                        // column as S->h [kfirst].  kfirst is not kept.
                        // A->nvec <= A->vdim == S->vdim
                        // A->hfirst == 0 (ignored)

        for (k = 0 ; k < A->nvec ; k++)
        {
            j = A->h [k] ;
            // operate on column A(:,j), which is also S (:,j)
            for (p = Ap [k] ; p < Ap [k+1] ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p].
                // This is identical to S(i,j)
            }
        }

    //--------------------
    // all of the above: via GBI_for_each_vector (A)
                        // are done with a single iterator that selects
                        // the iteration method based on the format of A.

        GBI_for_each_vector (A)
        {
            // get A(:,j)
            GBI_jth_iteration (j, pstart, pend) ;
            // operate on column A(:,j)
            for (p = pstart ; p < pend ; p++)
            {
                // A(i,j) has row i = Ai [p], value aij = Ax [p].
            }
        }

#endif

//------------------------------------------------------------------------------
// GBI_single_iterator: iterate over the vectors of a single matrix
//------------------------------------------------------------------------------

// The matrix may be sparse, hypersparse, slice, or hyperslice.

typedef struct
{
    const int64_t *GB_RESTRICT p ; // vector pointer A->p of A
    const int64_t *GB_RESTRICT h ; // A->h: hyperlist of vectors in A
    int64_t nvec ;              // A->nvec: number of vectors in A
    int64_t hfirst ;            // A->hfirst: first vector in slice A
    bool is_hyper ;             // true if A is hypersparse
    bool is_slice ;             // true if A is a slice or hyperslice

} GBI_single_iterator ;

//----------------------------------------
// GBI1_init: initialize a GBI_single_iterator
//----------------------------------------

static inline void GBI1_init
(
    GBI_single_iterator *Iter,
    const GrB_Matrix A
)
{ 
    // load the content of A into the iterator
    Iter->is_hyper = A->is_hyper ;
    Iter->p = A->p ;
    Iter->h = A->h ;
    Iter->nvec = A->nvec ;
    Iter->is_slice = A->is_slice ;
    Iter->hfirst = A->hfirst ;
}

//----------------------------------------
// GBI1_start: start the kth iteration for GBI_single_iterator
//----------------------------------------

static inline void GBI1_start
(
    int64_t Iter_k,
    GBI_single_iterator *Iter,
    int64_t *j,
    int64_t *pstart,
    int64_t *pend
)
{

    // get j: next vector from A
    if (Iter->is_slice)
    {
        if (Iter->is_hyper)
        {
            // A is a hyperslice of a hypersparse matrix
            (*j) = Iter->h [Iter_k] ;
        }
        else
        {
            // A is a slice of a standard matrix
            (*j) = (Iter->hfirst) + Iter_k ;
        }
    }
    else
    {
        if (Iter->is_hyper)
        { 
            // A is a hypersparse matrix
            (*j) = Iter->h [Iter_k] ;
        }
        else
        { 
            // A is a standard matrix
            (*j) = Iter_k ;
        }
    }

    // get the start and end of the next vector of A
    (*pstart) = Iter->p [Iter_k  ] ;
    (*pend)   = Iter->p [Iter_k+1] ;
}

// iterate over one matrix A (sparse, hypersparse, slice, or hyperslice)
// with a named iterator
#define GBI_for_each_vector_with_iter(Iter,A)                               \
    GBI_single_iterator Iter ;                                              \
    GBI1_init (&Iter, A) ;                                                  \
    for (int64_t Iter ## _k = 0 ; Iter ## _k < Iter.nvec ; Iter ## _k++)

// iterate over one matrix A (sparse, hypersparse, slice, or hyperslice)
// with the iterator named "Iter"
#define GBI_for_each_vector(A) GBI_for_each_vector_with_iter (Iter,A)

// get the column at the current iteration, and the start/end pointers
// of column j in the matrix A
#define GBI_jth_iteration_with_iter(Iter,j0,pstart0,pend0)                  \
    int64_t j0, pstart0, pend0 ;                                            \
    GBI1_start (Iter ## _k, &Iter, &j0, &pstart0, &pend0) ;

#define GBI_jth_iteration(j0,pstart0,pend0)                                 \
    GBI_jth_iteration_with_iter(Iter,j0,pstart0,pend0)

// iterate over a vector of a single matrix
#define GBI_for_each_entry(j,p,pend)                                        \
    GBI_jth_iteration (j, p, pend) ;                                        \
    for ( ; (p) < (pend) ; (p)++)

#endif

