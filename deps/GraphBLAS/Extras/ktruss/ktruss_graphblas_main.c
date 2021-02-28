//------------------------------------------------------------------------------
// ktruss_graphblas_main.c: find the k-truss of a graph using GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and find the k-truss
// Usage:
//
//  ktruss_graphblas_main   < infile
//  ktruss_graphblas_main 1 < infile
//  ktruss_graphblas_main 0 nrows ncols ntuples method
//  ktruss_graphblas_main 1 nx ny method
//
// Where infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The default file format is 0-based, but with "ktruss_main 1 < infile" the
// matrix is assumed to be 1-based.

// The dimensions of A are assumed to be the largest row and column indices,
// plus one if the matrix is 1-based.  This is done in get_matrix.c.
//
// For the second usage (ktruss_main 0 ...), a random symmetric matrix is
// created of size nrows-by-ncols with ntuples edges (some will be duplicates
// so the actual number of edges will be slightly less).  The method is 0 for
// setElement and 1 for build.  The matrix will not have any self-edges, which
// cause the method to fail.
//
// The 3rd usage (ktruss_main 1 ...) creates a finite-element matrix on an
// nx-by-ny grid.  Method is 0 to 3; refer to wathen.c for details.

#ifndef MATLAB_MEX_FILE

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                \
    GrB_free (&C) ;             \
    GrB_free (&A) ;

#include "ktruss_graphblas_def.h"

int main (int argc, char **argv)
{
    GrB_Matrix C = NULL, A = NULL ;
    GrB_Info info ;
    double tic [2] ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    printf ("--------------------------------------------------------------\n");

    //--------------------------------------------------------------------------
    // get a symmetric matrix with no self edges
    //--------------------------------------------------------------------------

    // get_matrix reads in a double-precision matrix.  It could easily be
    // changed to read in int64 matrix instead, but this would affect the
    // other GraphBLAS demos.  So the time to typecast A = (int64) C is added
    // to the read time, not the prep time for finding the k-truss.
    simple_tic (tic) ;
    OK (get_matrix (&C, argc, argv, true, false)) ;
    GrB_Index n, nedges ;
    OK (GrB_Matrix_nrows (&n, C)) ;
    // GxB_print (C, GxB_COMPLETE) ;

    // A = spones (C), and typecast to int64
    OK (GrB_Matrix_new (&A, GrB_INT64, n, n)) ;
    OK (GrB_apply (A, NULL, NULL, GxB_ONE_INT64, C, NULL)) ;
    double t_read = simple_toc (tic) ;
    printf ("\ntotal time to read A matrix: %14.6f sec\n", t_read) ;
    GrB_free (&C) ;
    OK (GrB_Matrix_nvals (&nedges, A)) ;
    nedges /= 2 ;

    // for further MATLAB analysis
    FILE *fm = fopen ("ktruss_grb_results.m", "a") ;
    fprintf (fm, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n") ;
    fprintf (fm, "id = id + 1 ;\n") ;
    fprintf (fm, "N (id) = %" PRIu64 " ;\n", n) ;
    fprintf (fm, "Nedges (id) = %" PRIu64 " ;\n", nedges) ;
    fprintf (fm, "%% Time (3:kmax) = time for k-truss\n") ;

    //--------------------------------------------------------------------------
    // find all k-trusses
    //--------------------------------------------------------------------------

    // for (int64_t k = 3 ; ; k++)
    int64_t k = 3 ;
    {

        //----------------------------------------------------------------------
        // find the k-truss
        //----------------------------------------------------------------------

        int64_t nsteps ;
        simple_tic (tic) ;
        OK (ktruss_graphblas (&C, A, k, &nsteps)) ;
        double t = simple_toc (tic) ;

        //----------------------------------------------------------------------
        // check result and free workspace
        //----------------------------------------------------------------------

        GrB_Index nnz ;
        OK (GrB_Matrix_nvals (&nnz, C)) ;
        GrB_Index ne = nnz / 2 ;

        int64_t nt ;
        OK (GrB_reduce (&nt, NULL, GxB_PLUS_INT64_MONOID, C, NULL)) ;
        nt /= 6 ;

        printf (
        "ktruss_grblas nthreads %3d : k %4"PRId64" ne %10" PRId64" nt %10" PRId64" %12.6f sec"
        " rate %7.2f steps %4" PRId64"\n", 1, k, ne, nt, t, 1e-6*nedges/t, nsteps) ;

        // print the entire k-truss if it's small
        /*
        #define NZMAX 400
        if (nnz > 0 && nnz < NZMAX)
        {
            printf ("\nEntire %lld-truss of the graph:\n", k) ;
            GrB_Index I [NZMAX], J [NZMAX] ;
            int64_t X [NZMAX] ;
            OK (GrB_Matrix_extractTuples (I, J, X, &nnz, C)) ;
            for (int p = 0 ; p < nnz ; p++)
            {
                if (I [p] < J [p])
                {
                    printf ("    (%lld,%lld): support %u\n",
                        I [p], J [p], X [p]) ;
                }
            }
        }
        */

        GrB_free (&C) ;
        fprintf (fm, "Time (%"PRId64") = %12.6g ;\n", k, t) ;

        // if (nnz == 0) break ;
    }

    fprintf (fm, "T {id} = Time' ;\n") ;
    fprintf (fm, "File {id} = filetrim (file) ;\n\n") ;
    fclose (fm) ;
    FREE_ALL ;
    GrB_finalize ( ) ;
}

#undef FREE_ALL
#endif
