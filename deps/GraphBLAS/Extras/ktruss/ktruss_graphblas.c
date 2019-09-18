//------------------------------------------------------------------------------
// ktruss_graphblas.c: find the k-truss subgraph of a graph via GraphBLAS
//------------------------------------------------------------------------------

// Given a symmetric graph A with no-self edges, ktruss_graphblas finds the
// k-truss subgraph of A.

// The edge weights of A are treated as binary.  Explicit zero entries in A are
// treated as non-edges.  Any type will work, but int64 is recommended for
// fastest results since that is the type used here for the semiring.
// GraphBLAS will do typecasting internally, but that takes extra time. 

// The output matrix C is the k-truss subgraph of A.  Its edges are a subset of
// A.  Each edge in C is part of at least k-2 triangles in C.  The pattern of C
// is the adjacency matrix of the k-truss subgraph of A.  The edge weights of C
// are the support of each edge.  That is, C(i,j)=nt if the edge (i,j) is part
// of nt triangles in C.  All edges in C have support of at least k-2.  The
// total number of triangles in C is reduce(C,'plus')/6.  The number of edges
// in C is nnz(C)/2.  C is returned as symmetric with a zero-free diagonal.

// Usage: constructs C as the k-truss of A
//      GrB_Matrix C = NULL ;
//      int64_t nsteps ;
//      GrB_Info info = ktruss_graphblas (&C, A, k, &nsteps) ;

// Compare this function with the MATLAB equivalent, ktruss.m.

// Modified for SuiteSparse:GraphBLAS V3.0:  support changed to a
// GrB_Vector, for input to GxB_select.

#define FREE_ALL                        \
    GrB_free (Support) ;                \
    GrB_free (&supportop) ;             \
    GrB_free (&C) ;

#include "ktruss_graphblas_def.h"

//------------------------------------------------------------------------------
// support_function:  select function for GxB_SelectOp and GxB_select
//------------------------------------------------------------------------------

bool support_function (const GrB_Index i, const GrB_Index j,
    const GrB_Index nrows, const GrB_Index ncols,
    const int64_t *x, const int64_t *support)
{
    return ((*x) >= (*support)) ;
}

//------------------------------------------------------------------------------
// C = ktruss_graphblas (A,k): find the k-truss subgraph of a graph
//------------------------------------------------------------------------------

GrB_Info ktruss_graphblas       // compute the k-truss of a graph
(
    GrB_Matrix *p_C,            // output k-truss subgraph, C
    GrB_Matrix A,               // input adjacency matrix, A, not modified
    const int64_t k,            // find the k-truss, where k >= 3
    int64_t *p_nsteps           // # of steps taken
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    // ensure k is 3 or more
    if (k < 3) return (GrB_INVALID_VALUE) ;

    if (p_C == NULL || p_nsteps == NULL) return (GrB_NULL_POINTER) ;

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    GrB_Info info ;
    GxB_SelectOp supportop = NULL ;
    GrB_Vector Support = NULL ;

    GrB_Index n ;
    GrB_Matrix C = NULL ;
    OK (GrB_Matrix_nrows (&n, A)) ;
    OK (GrB_Matrix_new (&C, GrB_INT64, n, n)) ;

    // select operator
    int64_t support = (k-2) ;
    OK (GxB_SelectOp_new (&supportop, support_function, GrB_INT64, GrB_INT64)) ;
    OK (GrB_Vector_new (&Support, GrB_INT64, 1)) ;
    OK (GrB_Vector_setElement (Support, support, 0)) ;

    // last_cnz = nnz (A)
    GrB_Index cnz, last_cnz ;
    OK (GrB_Matrix_nvals (&last_cnz, A)) ;

    //--------------------------------------------------------------------------
    // find the k-truss of A
    //--------------------------------------------------------------------------

    double tmult = 0 ;
    double tsel  = 0 ;

    for (int64_t nsteps = 1 ; ; nsteps++)
    {

        //----------------------------------------------------------------------
        // C<C> = C*C
        //----------------------------------------------------------------------

        GrB_Matrix Cin = (nsteps == 1) ? A : C ;
        double t1 = omp_get_wtime ( ) ;
        OK (GrB_mxm (C, Cin, NULL, GxB_PLUS_LAND_INT64, Cin, Cin, NULL)) ;
        double t2 = omp_get_wtime ( ) ;
        printf ("C<C>=C*C time: %g\n", t2-t1) ;
        tmult += (t2-t1) ;

        //----------------------------------------------------------------------
        // C = C .* (C >= support)
        //----------------------------------------------------------------------

        OK (GxB_select (C, NULL, NULL, supportop, C, Support, NULL)) ;

        double t3 = omp_get_wtime ( ) ;
        printf ("select time: %g\n", t3-t2) ;
        tsel += (t3-t2) ;

        //----------------------------------------------------------------------
        // check if the k-truss has been found
        //----------------------------------------------------------------------

        OK (GrB_Matrix_nvals (&cnz, C)) ;
        if (cnz == last_cnz)
        {
            printf ("ktruss_grb done: tmult %g tsel %g\n", tmult, tsel) ;
            (*p_C) = C ;                        // return the output matrix C
            (*p_nsteps) = nsteps ;              // return # of steps
            OK (GrB_free (&supportop)) ;        // free the select operator
            OK (GrB_free (Support)) ;           // free the select Thunk
            return (GrB_SUCCESS) ;
        }
        last_cnz = cnz ;
    }
}

#undef FREE_ALL

