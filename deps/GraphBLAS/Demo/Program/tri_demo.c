//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/tri_demo.c: count triangles
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and count the # of triangles using two methods.
// Usage:
//
//  tri_demo   < infile
//  tri_demo 1 < infile
//  tri_demo 0 nrows ncols ntuples method
//  tri_demo 1 nx ny method
//
// Where infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The default file format is 0-based, but with "tri_demo 1 < infile" the
// matrix is assumed to be 1-based.

// The dimensions of A are assumed to be the largest row and column indices,
// plus one if the matrix is 1-based.  This is done in read_matrix.c.
//
// For the second usage (tri_demo 0 ...), a random symmetric matrix is created
// of size nrows-by-ncols with ntuples edges (some will be duplicates so the
// actual number of edges will be slightly less).  The method is 0 for
// setElement and 1 for build.  The matrix will not have any self-edges, which
// cause the tricount method to fail.
//
// The 3rd usage (tri_demo 1 ...) creates a finite-element matrix on an
// nx-by-ny grid.  Method is 0 to 3; refer to wathen.c for details.

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                \
    GrB_free (&C) ;             \
    GrB_free (&A) ;             \
    GrB_free (&L) ;             \
    GrB_free (&U) ;

#include "demos.h"

int main (int argc, char **argv)
{
    GrB_Matrix C = NULL, A = NULL, L = NULL, U = NULL ;
    GrB_Info info ;
    double tic [2], r1, r2 ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    fprintf (stderr, "tri_demo:\n") ;
    printf ("--------------------------------------------------------------\n");

    //--------------------------------------------------------------------------
    // get a symmetric matrix with no self edges
    //--------------------------------------------------------------------------

    // get_matrix reads in a boolean matrix.  It could easily be changed to
    // read in uint32 matrix instead, but this would affect the other GraphBLAS
    // demos.  So the time to typecast A = (uint32) C is added to the read
    // time, not the prep time for triangle counting.
    simple_tic (tic) ;
    OK (get_matrix (&C, argc, argv, true, true)) ;
    GrB_Index n, nedges ;
    OK (GrB_Matrix_nrows (&n, C)) ;

    // A = spones (C), and typecast to uint32
    OK (GrB_Matrix_new (&A, GrB_UINT32, n, n)) ;
    OK (GrB_apply (A, NULL, NULL, GxB_ONE_UINT32, C, NULL)) ;
    double t_read = simple_toc (tic) ;
    printf ("\ntotal time to read A matrix: %14.6f sec\n", t_read) ;
    GrB_free (&C) ;

    // U = triu (A,1)
    simple_tic (tic) ;
    GrB_Index k = 1 ;
    OK (GrB_Matrix_new (&U, GrB_UINT32, n, n)) ;
    OK (GxB_select (U, NULL, NULL, GxB_TRIU, A, &k, NULL)) ;
    OK (GrB_Matrix_nvals (&nedges, U)) ;
    printf ("\nn %.16g # edges %.16g\n", (double) n, (double) nedges) ;
    double t_U = simple_toc (tic) ;
    printf ("U=triu(A) time:  %14.6f sec\n", t_U) ;

    //--------------------------------------------------------------------------
    // count the triangles via C<U> = L'*U (dot-produt)
    //--------------------------------------------------------------------------

    // L = tril (A,-1), for method 4
    printf ("\n------------------------------------- dot product method:\n") ;
    simple_tic (tic) ;
    OK (GrB_Matrix_new (&L, GrB_UINT32, n, n)) ;
    k = -1 ;
    OK (GxB_select (L, NULL, NULL, GxB_TRIL, A, &k, NULL)) ;
    double t_L = simple_toc (tic) ;
    printf ("L=tril(A) time:  %14.6f sec\n", t_L) ;
    OK (GrB_free (&A)) ;

    double t_dot [2] ;
    int64_t ntri2 ;
    OK (tricount (&ntri2, 5, NULL, NULL, L, U, t_dot)) ;

    printf ("# triangles %.16g\n", (double) ntri2) ;
    fprintf (stderr, "# triangles %.16g\n", (double) ntri2) ;

    printf ("\ntricount time:   %14.6f sec (dot product method)\n",
        t_dot [0] + t_dot [1]) ;
    printf ("tri+prep time:   %14.6f sec (incl time to compute L and U)\n",
        t_dot [0] + t_dot [1] + t_U + t_L) ;

    printf ("compute C time:  %14.6f sec\n", t_dot [0]) ;
    printf ("reduce (C) time: %14.6f sec\n", t_dot [1]) ;

    r1 = 1e-6*nedges / (t_dot [0] + t_dot [1] + t_U + t_L) ;
    r2 = 1e-6*nedges / (t_dot [0] + t_dot [1]) ;
    printf ("rate %10.2f million edges/sec (incl time for U=triu(A))\n", r1) ;
    printf ("rate %10.2f million edges/sec (just tricount itself)\n\n",  r2) ;
    fprintf (stderr, "GrB: C<U>=L'*U (dot)   "
            "rate %10.2f (with prep), %10.2f (just tricount)\n", r1, r2) ;

    //--------------------------------------------------------------------------
    // count the triangles via C<L> = L*L (outer-product)
    //--------------------------------------------------------------------------

    printf ("\n----------------------------------- outer product method:\n") ;

    double t_mark [2] = { 0, 0 } ;
    int64_t ntri1 ;
    OK (tricount (&ntri1, 3, NULL, NULL, L, NULL, t_mark)) ;

    printf ("tricount time:   %14.6f sec (outer product method)\n",
        t_mark [0] + t_mark [1]) ;
    printf ("tri+prep time:   %14.6f sec (incl time to compute L)\n",
        t_mark [0] + t_mark [1] + t_L) ;

    printf ("compute C time:  %14.6f sec\n", t_mark [0]) ;
    printf ("reduce (C) time: %14.6f sec\n", t_mark [1]) ;

    r1 = 1e-6*((double)nedges) / (t_mark [0] + t_mark [1] + t_L) ;
    r2 = 1e-6*((double)nedges) / (t_mark [0] + t_mark [1]) ;
    printf ("rate %10.2f million edges/sec (incl time for L=tril(A))\n", r1) ;
    printf ("rate %10.2f million edges/sec (just tricount itself)\n\n",  r2) ;
    fprintf (stderr, "GrB: C<L>=L*L (outer)  "
            "rate %10.2f (with prep), %10.2f (just tricount)\n", r1, r2) ;

    //--------------------------------------------------------------------------
    // check result and free workspace
    //--------------------------------------------------------------------------

    if (ntri1 != ntri2)
    {
        printf ("error! %.16g %.16g\n", (double) ntri1, (double) ntri2) ;
        exit (1) ;
    }
    FREE_ALL ;
    GrB_finalize ( ) ;
    printf ("\n") ;
    fprintf (stderr, "\n") ;
}

