//------------------------------------------------------------------------------
// allktruss_graphblas_main.c: find all k-trusses of a graph using GraphBLAS
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and find the k-truss
// Usage:
//
//  allktruss_graphblas_main   < infile
//  allktruss_graphblas_main 1 < infile
//  allktruss_graphblas_main 0 nrows ncols ntuples method
//  allktruss_graphblas_main 1 nx ny method
//
// Where infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The default file format is 0-based, but with "ktruss_main 1 < infile" the
// matrix is assumed to be 1-based.

// The dimensions of A are assumed to be the largest row and column indices,
// plus one if the matrix is 1-based.  This is done in read_matrix.c.
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
#define FREE_ALL                                            \
    if (Cset != NULL)                                       \
    {                                                       \
        for (int64_t k = 0 ; k < n+1 ; k++)                 \
        {                                                   \
            if (Cset [k] != NULL) GrB_free (&(Cset [k])) ;  \
        }                                                   \
    }                                                       \
    if (ntris != NULL) free (ntris) ;                       \
    if (nedges != NULL) free (nedges) ;                     \
    if (nstepss != NULL) free (nstepss) ;                   \
    GrB_free (&T) ;                                         \
    GrB_free (&A) ;

#include "ktruss_graphblas_def.h"

int main (int argc, char **argv)
{
    GrB_Index n = 0, anedges ;
    GrB_Matrix T = NULL, A = NULL, *Cset = NULL ;
    GrB_Info info ;
    double tic [2] ;

    int64_t kmax ;              // smallest k where k-truss is empty
    int64_t *ntris = NULL ;     // size n, ntris [k] is #triangles in k-truss
    int64_t *nedges = NULL ;    // size n, nedges [k] is #edges in k-truss
    int64_t *nstepss = NULL ;   // size n, nsteps [k] is #steps for k-truss

    GrB_init (GrB_NONBLOCKING) ;
    printf ("--------------------------------------------------------------\n");

    //--------------------------------------------------------------------------
    // get a symmetric matrix with no self edges
    //--------------------------------------------------------------------------

    // get_matrix reads in a double-precision matrix.  It could easily be
    // changed to read in int64 matrix instead, but this would affect the
    // other GraphBLAS demos.  So the time to typecast A = (int64) T is added
    // to the read time, not the prep time for finding the k-truss.
    simple_tic (tic) ;
    OK (get_matrix (&T, argc, argv, true, false)) ;
    OK (GrB_Matrix_nrows (&n, T)) ;

    // A = spones (T), and typecast to int64
    OK (GrB_Matrix_new (&A, GrB_INT64, n, n)) ;
    OK (GrB_apply (A, NULL, NULL, GxB_ONE_INT64, T, NULL)) ;
    double t_read = simple_toc (tic) ;
    printf ("\ntotal time to read A matrix: %14.6f sec\n", t_read) ;
    GrB_free (&T) ;
    OK (GrB_Matrix_nvals (&anedges, A)) ;
    anedges /= 2 ;

    ntris   = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;
    nedges  = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;
    nstepss = (int64_t *) malloc ((n+1) * sizeof (int64_t)) ;

    if (ntris == NULL || nedges == NULL || nstepss == NULL)
    {
        FREE_ALL ;
        printf ("out of memory\n") ;
        exit (1) ;
    }

    //--------------------------------------------------------------------------
    // find all k-trusses
    //--------------------------------------------------------------------------

    // for further MATLAB analysis
    FILE *fm = fopen ("allktruss_grb_results.m", "a") ;
    fprintf (fm, "\n%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%\n") ;
    fprintf (fm, "id = id + 1 ;\n") ;
    fprintf (fm, "N (id) = %" PRIu64 ";\n", n) ;
    fprintf (fm, "Nedges (id) = %" PRIu64 " ;\n", anedges) ;
    fprintf (fm, "%% Tgb (keep, id) = time for all-k-truss\n") ;

    // for (int keep = 0 ; keep <= 1 ; keep++)
    int keep = 0 ;
    {
        printf ("\nkeep: %d\n", keep) ;
        if (keep)
        {
            // construct and keep all k-trusses
            Cset = (GrB_Matrix *) calloc (n+1, sizeof (GrB_Matrix)) ;
            if (Cset == NULL)
            {
                FREE_ALL ;
                printf ("out of memory\n") ;
                exit (1) ;
            }
        }

        simple_tic (tic) ;
        OK (allktruss_graphblas (Cset, A, &kmax, ntris, nedges, nstepss)) ;
        double t = simple_toc (tic) ;

        if (keep == 0) fprintf (fm, "Kmax (id) = %" PRId64 " ;\n", kmax) ;
        fprintf (fm, "Tgb (%d,id) = %12.6g ;\n", 1+keep,t) ;

        for (int64_t k = 3 ; k <= kmax ; k++)
        {
            int64_t nt = ntris [k] ;
            int64_t ne = nedges [k] ;
            int64_t nsteps = nstepss [k] ;
            printf ("allktruss_grblas : "
                "k %4"PRId64" ne %10"PRId64" nt %10"PRId64" steps %4"PRId64"\n",
                k, ne, nt, nsteps) ;
            if (keep && Cset [k] != NULL)
            {
                GrB_Index cnz = 0 ;
                GrB_Matrix_nvals (&cnz, Cset [k]) ;
                if (cnz != 2*ne) { printf ("!!!\n") ; exit (1) ; }
            }
        }
        printf ("allktruss graphblas : %12.6f sec, rate %7.2f keep: %d\n",
            t, (kmax-2) * 1e-6*anedges/t, keep) ;
    }

    fprintf (fm, "File {id} = filetrim (file) ;\n\n") ;
    fclose (fm) ;
    FREE_ALL ;
    GrB_finalize ( ) ;
}

#undef FREE_ALL
#endif

