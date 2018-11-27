//------------------------------------------------------------------------------
// GraphBLAS/Demo/Source/tricount.c: count the number of triangles in a graph
//------------------------------------------------------------------------------

// Given a symmetric graph A with no-self edges, tricount counts the exact
// number of triangles in the graph.

// One of 5 methods are used.  Each computes the same result, ntri:

//  0:  minitri:    ntri = nnz (A*E == 2) / 3
//  1:  Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6
//  2:  Cohen:      ntri = sum (sum ((L * U) .* A)) / 2
//  3:  Sandia:     ntri = sum (sum ((U * U) .* U))
//  4:  Sandia2:    ntri = sum (sum ((L * L) .* L))
//  5:  SandiaDot:  ntri = sum (sum ((L * U') .* L)).  Note that L=U'.

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
// but uint32 is recommended for fastest results since that is the type used
// here for the semiring.  GraphBLAS will do typecasting internally, but that
// takes extra time. 

// This method has been updated as of Version 2.2 of SuiteSparse:GraphBLAS.  It
// now assumes the matrix is held by row (GxB_BY_ROW), not by column
// (GxB_BY_COL).  Both methods work fine, but with matrices stored by column,
// C<M>=A'*B uses the dot product method by default, whereas C<M>=A*B' uses the
// dot product method if the matrices are stored by row.

#define FREE_ALL                \
    GrB_free (&Two) ;           \
    GrB_free (&d) ;             \
    GrB_free (&S) ;             \
    GrB_free (&C) ;

#include "demos.h"

//------------------------------------------------------------------------------
// two:  unary function for GrB_apply
//------------------------------------------------------------------------------

void two (uint32_t *z, const uint32_t *x)
{
    (*z) = (double) (((*x) == 2) ? 1 : 0) ;
}

//------------------------------------------------------------------------------
// tricount: count the number of triangles in a graph
//------------------------------------------------------------------------------

GrB_Info tricount           // count # of triangles
(
    int64_t *p_ntri,        // # of trianagles
    const int method,       // 0 to 5, see above
    const GrB_Matrix A,     // adjacency matrix for methods 0, 1, and 2
    const GrB_Matrix E,     // edge incidence matrix for method 0
    const GrB_Matrix L,     // L=tril(A) for methods 2, 4, and 4
    const GrB_Matrix U,     // U=triu(A) for methods 2, 3, and 5
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

    switch (method)
    {
        case 0:  // minitri:    ntri = nnz (A*E == 2) / 3

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_ncols (&ne, E)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, ne)) ;
            // mxm:  outer product method, no mask
            OK (GrB_mxm (C, NULL, NULL, GxB_PLUS_TIMES_UINT32, A, E, NULL));
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_UnaryOp_new (&Two, two, GrB_UINT32, GrB_UINT32)) ;
            OK (GrB_Matrix_new (&S, GrB_UINT32, n, ne)) ;
            OK (GrB_apply (S, NULL, NULL, Two, C, NULL)) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, S, NULL)) ;
            ntri /= 3 ;
            break ;

        case 1:  // Burkhardt:  ntri = sum (sum ((A^2) .* A)) / 6

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GrB_mxm (C, A, NULL, GxB_PLUS_TIMES_UINT32, A, A, NULL)) ;
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
            ntri /= 6 ;
            break ;

        case 2:  // Cohen:      ntri = sum (sum ((L * U) .* A)) / 2

            OK (GrB_Matrix_nrows (&n, A)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GrB_mxm (C, A, NULL, GxB_PLUS_TIMES_UINT32, L, U, NULL)) ;
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
            ntri /= 2 ;
            break ;

        case 3:  // Sandia:    ntri = sum (sum ((L * L) .* L))

            OK (GrB_Matrix_nrows (&n, L)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;
            OK (GrB_mxm (C, L, NULL, GxB_PLUS_TIMES_UINT32, L, L, d)) ;
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
            break ;

        case 4:  // Sandia2:    ntri = sum (sum ((U * U) .* U))

            OK (GrB_Matrix_nrows (&n, U)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;
            // mxm:  outer product method, with mask
            OK (GrB_mxm (C, U, NULL, GxB_PLUS_TIMES_UINT32, U, U, NULL)) ;
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
            break ;

        case 5:  // SandiaDot:  ntri = sum (sum ((L * U') .* L))

            OK (GrB_Matrix_nrows (&n, U)) ;
            OK (GrB_Matrix_new (&C, GrB_UINT32, n, n)) ;
            OK (GrB_Descriptor_new (&d)) ;
            OK (GxB_set (d, GrB_INP1, GrB_TRAN)) ;
            // mxm:  dot product method, with mask
            OK (GrB_mxm (C, L, NULL, GxB_PLUS_TIMES_UINT32, L, U, d)) ;
            t [0] = simple_toc (tic) ;

            simple_tic (tic) ;
            OK (GrB_reduce (&ntri, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
            break ;

        default:    // invalid method

            return (GrB_INVALID_VALUE) ;
            break ;
    }

    FREE_ALL ;
    t [1] = simple_toc (tic) ;
    (*p_ntri) = ntri ;
    return (GrB_SUCCESS) ;
}

