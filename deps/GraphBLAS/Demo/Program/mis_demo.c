//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/mis_demo.c: maximal independent set
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and compute a maximal independent set. Usage:
//
//  mis_demo < infile
//  mis_demo 0 nrows ncols ntuples method
//  mis_demo 1 nx ny method
//
// Where infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The dimensions of A are assumed to be the largest row and column indices,
// plus one (in read_matrix.c).
//
// For the second usage (mis_demo 0 ...), a random symmetric matrix is created
// of size nrows-by-ncols with ntuples edges (some will be duplicates so the
// actual number of edges will be slightly less).  The method is 0 for
// setElement and 1 for build.  The matrix will not have any self-edges, which
// cause the mis method to fail.
//
// The 3rd usage (mis_demo 1 ...) creates a finite-element matrix on an
// nx-by-ny grid.  Method is 0 to 3; refer to wathen.c for details.

#include "demos.h"

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                \
    GrB_free (&A) ;             \
    GrB_free (&C) ;             \
    GrB_free (&iset1) ;         \
    GrB_free (&iset2) ;         \
    GrB_free (&e) ;             \
    GrB_free (&dt) ;            \
    GrB_free (&Lor) ;           \
    GrB_free (&Boolean) ;       \
    if (I2 != NULL) free (I2) ; \
    if (J2 != NULL) free (J2) ; \
    if (X2 != NULL) free (X2) ; \
    if (I != NULL) free (I) ;   \
    if (X != NULL) free (X) ;

int64_t isize1, isize2 ;
GrB_Index n ;
GrB_Matrix A = NULL, C = NULL ;
GrB_Vector iset1 = NULL, iset2 = NULL, e = NULL ;
GrB_Descriptor dt = NULL ;
GrB_Index *I = NULL, *I2 = NULL, *J2 = NULL ;
float *X = NULL ;
bool *X2 = NULL ;
GrB_Monoid Lor = NULL ;
GrB_Semiring Boolean = NULL ;
GrB_Info info ;

//------------------------------------------------------------------------------
// mis_check_results: test if iset is a maximal independent set
//------------------------------------------------------------------------------

GrB_Info mis_check_results
(
    int64_t *p_isize,
    GrB_Vector iset,
    double t
)
{

    //--------------------------------------------------------------------------
    // report the results
    //--------------------------------------------------------------------------

    int64_t isize ;
    OK (GrB_reduce (&isize, NULL, GxB_PLUS_INT64_MONOID, iset, NULL)) ;
    (*p_isize) = isize ;

    GrB_Index nvals ;
    OK (GrB_Vector_nvals (&nvals, iset)) ;
    I = malloc (nvals * sizeof (GrB_Index)) ;
    X = malloc (nvals * sizeof (float)) ;

    if (I == NULL || X == NULL)
    {
        printf ("out of memory\n") ;
        FREE_ALL ;
        exit (1) ;
    }

    OK (GrB_Vector_extractTuples (I, X, &nvals, iset)) ;

    // I [0..isize-1] is the independent set
    isize = 0 ;
    for (int64_t k = 0 ; k < nvals ; k++)
    {
        if (X [k]) 
        {
            // printf ("I [%lld] = %lld\n", isize, I [k]) ;
            I [isize++] = I [k] ;
        }
    }

    free (X) ; X = NULL ;

    printf ("independent set found: %.16g of %.16g nodes\n",
        (double) isize, (double) n) ;

    //--------------------------------------------------------------------------
    // verify the result
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&C, GrB_BOOL, isize, isize)) ;
    OK (GrB_extract (C, NULL, NULL, A, I, isize, I, isize, NULL)) ;
    OK (GrB_Matrix_nvals (&nvals, C)) ;

    I2 = malloc (nvals * sizeof (GrB_Index)) ;
    J2 = malloc (nvals * sizeof (GrB_Index)) ;
    X2 = malloc (nvals * sizeof (bool)) ;
    if (I2 == NULL || J2 == NULL || X2 == NULL)
    {
        printf ("out of memory\n") ;
        FREE_ALL ;
        exit (1) ;
    }

    // could do this with a mask instead of extractTuples.
    OK (GrB_Matrix_extractTuples (I2, J2, X2, &nvals, C)) ;
    GrB_free (&C) ;

    for (int64_t k = 0 ; k < nvals ; k++)
    {
        if (X2 [k] && I2 [k] != J2 [k])
        {
            printf ("error!  A(I,I) has an edge!\n") ;
            FREE_ALL ;
            exit (1) ;
        }
    }

    free (I2) ; I2 = NULL ;
    free (J2) ; J2 = NULL ;
    free (X2) ; X2 = NULL ;

    // now check if all other nodes are adjacent to the iset

    // e = iset
    OK (GrB_Vector_dup (&e, iset)) ;
    // e = (e || A*iset), using the Boolean semiring
    OK (GrB_vxm (e, NULL, GrB_LOR, GxB_LOR_LAND_BOOL, iset, A, NULL)) ;
    OK (GrB_Vector_nvals (&nvals, e)) ;
    GrB_free (&e) ;
    if (nvals != n)
    {
        fprintf (stderr, "error! A (I,I is not maximal!\n") ;
        exit (1) ;
    }

    free (I) ; I = NULL ;

    fprintf (stderr, "maximal independent set OK: %.16g of %.16g nodes"
        " time: %g\n", (double) isize, (double) n, t) ;
    return (GrB_SUCCESS) ;
}

//------------------------------------------------------------------------------
// mis_demo main
//------------------------------------------------------------------------------

int main (int argc, char **argv)
{

    double tic [2], t1, t2 ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    fprintf (stderr, "\nmis_demo:\n") ;

    //--------------------------------------------------------------------------
    // get a symmetric matrix with no self edges
    //--------------------------------------------------------------------------

    OK (get_matrix (&A, argc, argv, true, true)) ;
    OK (GrB_Matrix_nrows (&n, A)) ;

    //--------------------------------------------------------------------------
    // convert A to boolean
    //--------------------------------------------------------------------------

    OK (GrB_Descriptor_new (&dt)) ;
    OK (GxB_set (dt, GrB_INP0, GrB_TRAN)) ;
    OK (GrB_Matrix_new (&C, GrB_BOOL, n, n)) ;
    OK (GrB_transpose (C, NULL, NULL, A, dt)) ;
    GrB_free (&dt) ;
    GrB_free (&A) ;
    A = C ;
    C = NULL ;

    //--------------------------------------------------------------------------
    // compute maximal independent set
    //--------------------------------------------------------------------------

    for (int64_t seed = 1 ; seed <= 2 ; seed++)
    {

        simple_tic (tic) ;
        OK (mis (&iset1, A, seed)) ;
        t1 = simple_toc (tic) ;
        printf ("MIS time in seconds: %14.6f\n", t1) ;

        // also test the version that checks every error condition
        simple_tic (tic) ;
        OK (mis_check (&iset2, A, seed)) ;
        t2 = simple_toc (tic) ;
        printf ("MIS time in seconds: %14.6f\n", t2) ;

        //----------------------------------------------------------------------
        // compare results
        //----------------------------------------------------------------------

        mis_check_results (&isize1, iset1, t1) ;
        mis_check_results (&isize2, iset2, t2) ;

        printf ("isize: %.16g %.16g\n", (double) isize1, (double) isize2) ;

        if (isize1 != isize2)
        {
            fprintf (stderr, "=============================================="
                "======size differs!\n") ;
            printf ("size differs!\n") ;
        }
        GrB_free (&iset1) ;
        GrB_free (&iset2) ;
    }

    FREE_ALL ;
    GrB_finalize ( ) ;
}

