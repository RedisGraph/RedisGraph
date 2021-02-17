//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/tricount.c: count the number of triangles in a graph
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

//------------------------------------------------------------------------------

// Given a symmetric graph A with no-self edges, tricount counts the exact
// number of triangles in the graph.

// One of 5 methods are used.  Each computes the same result, ntri:

//  0:  minitri:    ntri = nnz (A*E == 2) / 3
//  1:  Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6
//  2:  Cohen:      ntri = sum (sum ((L * U) .* A)) / 2
//  3:  Sandia:     ntri = sum (sum ((L * L) .* L))
//  4:  Sandia2:    ntri = sum (sum ((U * U) .* U))
//  5:  SandiaDot:  ntri = sum (sum ((L * U') .* L)).  Note that L=U'.
//  6:  SandiaDot2: ntri = sum (sum ((U * L') .* U))

// All matrices are assumed to be in CSR format (GxB_BY_ROW).

// Method 0 can take a huge amount of memory, for all of A*E.  As a result,
// it often fails for large problems.

// Methods 1 and 2 are much more memory efficient as compare to Method 0,
// taking memory space the same size as A.  But they are slower than methods 3
// and 4.

// Methods 3 and 4 take a little less memory than methods 1 and 2, are by far
// the fastest methods in general.  The two methods compute the same
// intermediate matrix (U*U), and differ only in the way the matrix
// multiplication is done.  Method 3 uses an outer-product method (Gustavson's
// method).  Method 5 uses dot products and does not explicitly transpose U.
// They are called the "Sandia" method since matrices in the  KokkosKernels
// are stored in compressed-sparse row form, so (L*L).*L in the KokkosKernel
// method is equivalent to (L*L).*L in SuiteSparse:GraphBLAS when the matrices
// in SuiteSparse:GraphBLAS are in their default format (also by row).

// A is a binary square symmetric matrix.  E is the edge incidence matrix of A.
// L=tril(A), and U=triu(A).  See GraphBLAS/Demo/tricount.m for a complete
// definition of each method and the matrices A, E, L, and U, and citations of
// relevant references.

// All input matrices should have binary values (0 and 1).  Any type will work,
// but int32 is recommended for fastest results since that is the type used
// here for the semiring.  GraphBLAS will do typecasting internally, but that
// takes extra time. 

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.  It
// now assumes the matrix is held by row (GxB_BY_ROW), not by column
// (GxB_BY_COL).  Both methods work fine, but with matrices stored by column,
// C<M>=A'*B uses the dot product method by default, whereas C<M>=A*B' uses the
// dot product method if the matrices are stored by row.

#include "GraphBLAS.h"

#define FREE_ALL                \
    GrB_UnaryOp_free (&Two) ;   \
    GrB_Descriptor_free (&d) ;  \
    GrB_Matrix_free (&S) ;      \
    GrB_Matrix_free (&C) ;

#undef GB_PUBLIC
#define GB_LIBRARY
#include "graphblas_demos.h"

//------------------------------------------------------------------------------
// two:  unary function for GrB_apply
//------------------------------------------------------------------------------

void two (int32_t *z, const int32_t *x)
{
    (*z) = (double) (((*x) == 2) ? 1 : 0) ;
}

//------------------------------------------------------------------------------
// tricount: count the number of triangles in a graph
//------------------------------------------------------------------------------

GB_PUBLIC
GrB_Info tricount           // count # of triangles
(
    int64_t *p_ntri,        // # of trianagles
    const int method,       // 0 to 6, see above
    const GrB_Matrix A,     // adjacency matrix for methods 0, 1, and 2
    const GrB_Matrix E,     // edge incidence matrix for method 0
    const GrB_Matrix L,     // L=tril(A) for methods 2, 3, 5, and 6
    const GrB_Matrix U,     // U=triu(A) for methods 2, 4, 5, and 6
    double t [2]            // t [0]: multiply time, t [1]: reduce time
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    double tic [2] ;
    simple_tic (tic) ;
    GrB_Info info ;
    int64_t ntri ;
    GrB_Index n, ne ;
    GrB_UnaryOp Two = NULL ;
    GrB_Matrix S = NULL, C = NULL ;
    GrB_Descriptor d = NULL ;
    OK (GrB_Descriptor_new (&d)) ;

    GrB_Semiring semiring = GrB_PLUS_TIMES_SEMIRING_INT32 ;
    GrB_Type ctype = GrB_INT32 ;

    switch (method)
    {
        case 0:  // minitri:    ntri = nnz (A*E == 2) / 3

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_ncols (&ne, E)) ;
            OK (GrB_Matrix_new (&C, ctype, n, ne)) ;
            // mxm:  outer product method, no mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
            OK (GrB_mxm (C, NULL, NULL, GxB_PLUS_TIMES_UINT32, A, E, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_UnaryOp_new (&Two, (GxB_unary_function) two, ctype, ctype));
            OK (GrB_Matrix_new (&S, ctype, n, ne)) ;
            OK (GrB_Matrix_apply (S, NULL, NULL, Two, C, NULL)) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                S, NULL)) ;
            ntri /= 3 ;
            break ;

        case 1:  // Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
            OK (GrB_mxm (C, A, NULL, semiring, A, A, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            ntri /= 6 ;
            break ;

        case 2:  // Cohen:      ntri = sum (sum ((L * U) .* A)) / 2

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
            OK (GrB_mxm (C, A, NULL, semiring, L, U, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            ntri /= 2 ;
            break ;

        case 3:  // Sandia:    ntri = sum (sum ((L * L) .* L))

            OK (GrB_Matrix_nrows (&n, L)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
            OK (GrB_mxm (C, L, NULL, semiring, L, L, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            break ;

        case 4:  // Sandia2:    ntri = sum (sum ((U * U) .* U))

            OK (GrB_Matrix_nrows (&n, U)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_GUSTAVSON)) ;
            OK (GrB_mxm (C, U, NULL, semiring, U, U, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            break ;

        case 5:  // SandiaDot:  ntri = sum (sum ((L * U') .* L))

            OK (GrB_Matrix_nrows (&n, U)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            OK (GxB_Desc_set (d, GrB_INP1, GrB_TRAN)) ;
            // mxm:  dot product method, with mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_DOT)) ;
            OK (GrB_mxm (C, L, NULL, semiring, L, U, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            break ;

        case 6:  // SandiaDot2: ntri = sum (sum ((U * L') .* U))

            OK (GrB_Matrix_nrows (&n, U)) ;
            OK (GrB_Matrix_new (&C, ctype, n, n)) ;
            OK (GxB_Desc_set (d, GrB_INP1, GrB_TRAN)) ;
            // mxm:  dot product method, with mask
            OK (GxB_Desc_set (d, GxB_AxB_METHOD, GxB_AxB_DOT)) ;
            OK (GrB_mxm (C, U, NULL, semiring, U, L, d)) ;
            t [0] = simple_toc (tic) ;
            simple_tic (tic) ;
            OK (GrB_Matrix_reduce_INT64 (&ntri, NULL, GrB_PLUS_MONOID_INT64,
                C, NULL)) ;
            break ;

        default:    // invalid method

            FREE_ALL ;
            return (GrB_INVALID_VALUE) ;
            break ;
    }

    FREE_ALL ;
    t [1] = simple_toc (tic) ;
    (*p_ntri) = ntri ;
    return (GrB_SUCCESS) ;
}

