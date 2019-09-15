//------------------------------------------------------------------------------
// allktruss_graphblas.c: find all k-trusses of a graph via GraphBLAS
//------------------------------------------------------------------------------

// Given a symmetric graph A with no-self edges, ktruss_graphblas finds all
// k-trusses of A.

// The edge weights of A are treated as binary.  Explicit zero entries in A are
// treated as non-edges.  Any type will work, but int64 is recommended for
// fastest results since that is the type used here for the semiring.
// GraphBLAS will do typecasting internally, but that takes extra time. 

// The optional output matrices Cset [3..kmax-1] are the k-trusses of A.  Their
// edges are a subset of A.  Each edge in C = Cset [k] is part of at least k-2
// triangles in C.  The pattern of C is the adjacency matrix of the k-truss
// subgraph of A.  The edge weights of C are the support of each edge.  That
// is, C(i,j)=nt if the edge (i,j) is part of nt triangles in C.  All edges in
// C have support of at least k-2.  The total number of triangles in C is
// reduce(C,'plus')/6.  The number of edges in C is nnz(C)/2.  C is returned as
// symmetric with a zero-free diagonal.  The k-trusses are not returned if Cset
// is NULL.  Cset [kmax] is NULL since the kmax-truss is empty.

// Usage: constructs k-trusses of A, for k = 3:kmax
//      GrB_Info info = allktruss_graphblas (&Cset, A, &kmax,
//          ntries, nedges, nstepss) ;

// Compare this function with the MATLAB equivalent, allktruss.m.

// Modified for SuiteSparse:GraphBLAS V3.0:  support changed to a
// GrB_Vector, for input to GxB_select.

#define FREE_ALL                                \
    if (keep_all_ktrusses)                      \
    {                                           \
        for (int64_t kk = 3 ; kk <= k ; kk++)   \
        {                                       \
            GrB_free (&(Cset [kk])) ;           \
        }                                       \
    }                                           \
    GrB_free (Support) ;                        \
    GrB_free (&supportop) ;                     \
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
// C = allktruss_graphblas (A,k): find all k-trusses a graph
//------------------------------------------------------------------------------

GrB_Info allktruss_graphblas    // compute all k-trusses of a graph
(
    GrB_Matrix *Cset,           // output k-truss subgraphs (optional)
    GrB_Matrix A,               // input adjacency matrix, A, not modified

    // output statistics
    int64_t *kmax,              // smallest k where k-truss is empty
    int64_t *ntris,             // size n, ntris [k] is #triangles in k-truss
    int64_t *nedges,            // size n, nedges [k] is #edges in k-truss
    int64_t *nstepss            // size n, nstepss [k] is #steps for k-truss
)
{

    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    if (nstepss == NULL || kmax == NULL || ntris == NULL || nedges == NULL)
    {
        return (GrB_NULL_POINTER) ;
    }

    //--------------------------------------------------------------------------
    // initializations
    //--------------------------------------------------------------------------

    bool keep_all_ktrusses = (Cset != NULL) ;

    int64_t k ;
    for (k = 0 ; k < 3 ; k++)
    {
        if (keep_all_ktrusses)
        {
            Cset [k] = NULL ;
        }
        ntris   [k] = 0 ;
        nedges  [k] = 0 ;
        nstepss [k] = 0 ;
    }
    (*kmax) = 0 ;
    k = 0 ;

    GrB_Info info ;

    // the current k-truss
    GrB_Matrix C = NULL ;

    // select operator
    GxB_SelectOp supportop = NULL ;
    GrB_Vector Support = NULL ;

    // get the size of A
    GrB_Index n ;
    OK (GrB_Matrix_nrows (&n, A)) ;

    // create a select operator for GxB_select
    OK (GxB_SelectOp_new (&supportop, support_function, GrB_INT64, GrB_INT64)) ;
    OK (GrB_Vector_new (&Support, GrB_INT64, 1)) ;

    //--------------------------------------------------------------------------
    // C<A> = A*A
    //--------------------------------------------------------------------------

    double tmult = 0 ;
    double tsel  = 0 ;
    double t1 = omp_get_wtime ( ) ;

    GrB_Index last_cnz ;
    OK (GrB_Matrix_nvals (&last_cnz, A)) ;       // last_cnz = nnz (A)
    OK (GrB_Matrix_new (&C, GrB_INT64, n, n)) ;
    OK (GrB_mxm (C, A, NULL, GxB_PLUS_LAND_INT64, A, A, NULL)) ;
    int64_t nsteps = 1 ;

    double t2 = omp_get_wtime ( ) ;
    printf ("cmult time: %g\n", t2-t1) ;
    tmult += (t2-t1) ;

    //--------------------------------------------------------------------------
    // find all k-trusses
    //--------------------------------------------------------------------------

    for (k = 3 ; ; k++)
    {

        //----------------------------------------------------------------------
        // find the k-truss
        //----------------------------------------------------------------------

        int64_t support = (k-2) ;
        OK (GrB_Vector_setElement (Support, support, 0)) ;

        while (1)
        {

            //------------------------------------------------------------------
            // C = C .* (C >= support)
            //------------------------------------------------------------------

            double t1 = omp_get_wtime ( ) ;

            OK (GxB_select (C, NULL, NULL, supportop, C, Support, NULL)) ;

            t2 = omp_get_wtime ( ) ;
            printf ("select time: %g\n", t2-t1) ;
            tsel += (t2-t1) ;

            //------------------------------------------------------------------
            // check if k-truss has been found
            //------------------------------------------------------------------

            GrB_Index cnz ;
            OK (GrB_Matrix_nvals (&cnz, C)) ;
            if (cnz == last_cnz)
            {
                // k-truss has been found
                int64_t nt = 0 ;
                OK (GrB_reduce (&nt, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
                ntris   [k] = nt / 6 ;
                nedges  [k] = cnz / 2 ;
                nstepss [k] = nsteps ;
                nsteps = 0 ;
                if (cnz == 0)
                {
                    // this is the last k-truss
                    OK (GrB_free (&supportop)) ;    // free the select operator
                    OK (GrB_free (Support)) ;       // free the select Thunk
                    OK (GrB_free (&C)) ;            // free last empty k-truss
                    (*kmax) = k ;
                    if (keep_all_ktrusses)
                    {
                        Cset [k] = NULL ;
                    }
                    printf ("allktruss graphblas done: tmult %g tsel %g\n",
                        tmult, tsel) ;
                    return (GrB_SUCCESS) ;
                }
                else if (keep_all_ktrusses)
                {
                    // save the k-truss in the list of output k-trusses
                    OK (GrB_Matrix_dup (&(Cset [k]), C)) ;
                }
                // start finding the next k-truss
                break ;
            }

            // continue searching for this k-truss
            last_cnz = cnz ;
            nsteps++ ;

            //------------------------------------------------------------------
            // C<C> = C*C
            //------------------------------------------------------------------

            t1 = omp_get_wtime ( ) ;

            OK (GrB_mxm (C, C, NULL, GxB_PLUS_LAND_INT64, C, C, NULL)) ;

            t2 = omp_get_wtime ( ) ;
            printf ("mult time: %g\n", t2-t1) ;
            tmult += (t2-t1) ;
        }
    }
}

#undef FREE_ALL

