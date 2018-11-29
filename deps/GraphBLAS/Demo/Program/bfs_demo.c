//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/bfs_demo.c: breadth first search using vxm with a mask
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and performs a BFS using four different methods.

// Usage:
//
//  bfs_demo < infile
//  bfs_demo 0 nrows ncols ntuples method
//  bfs_demo 1 nx ny method
//
// Where infile has one line per edge in the graph; these have the form
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The dimensions of A are assumed to be the largest row and column indices,
// plus one (in read_matrix.c).
//
// For the second usage (bfs_demo 0 ...), a random symmetric matrix is created
// of size nrows-by-ncols with ntuples edges (some will be duplicates so the
// actual number of edges will be slightly less).  The method is 0 for
// setElement and 1 for build.
//
// The 3rd usage (bfs_demo 1 ...) creates a finite-element matrix on an
// nx-by-ny grid.  Method is 0 to 3; refer to wathen.c for details.

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL            \
    GrB_free (&v) ;         \
    GrB_free (&A) ;         \
    GrB_free (&Abool) ;     \
    GrB_free (&is_reachable) ; \
    GrB_free (&max_monoid) ;

#include "demos.h"

int main (int argc, char **argv)
{
    GrB_Info info ;
    GrB_Matrix A = NULL ;
    GrB_Matrix A2, Abool = NULL ;
    GrB_Vector v = NULL ;
    GrB_Vector is_reachable = NULL ;
    GrB_Monoid max_monoid = NULL ;
    double tic [2], t ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    fprintf (stderr, "bfs_demo:\n") ;

    //--------------------------------------------------------------------------
    // read a matrix from stdin
    //--------------------------------------------------------------------------

    // self edges are OK
    OK (get_matrix (&A, argc, argv, false, true)) ;
    GrB_Index n ;
    OK (GrB_Matrix_nrows (&n, A)) ;

    printf ("number of nodes: %g\n", (double) n) ;

    // typecast A to boolean, if needed.  This is not required but it
    // speeds up the BFS
    A2 = A ;

    GrB_Type atype ;
    OK (GxB_Matrix_type (&atype, A)) ;
    if (atype != GrB_BOOL)
    {
        OK (GrB_Matrix_new (&Abool, GrB_BOOL, n, n)) ;
        OK (GrB_apply (Abool, NULL, NULL, GrB_IDENTITY_BOOL, A, NULL)) ;
        A2 = Abool ;
    }

    for (int method = 0 ; method <= 3 ; method++)
    {

        //----------------------------------------------------------------------
        // do the BFS, starting at node zero
        //----------------------------------------------------------------------

        // All methods give identical results, just using different methods

        GrB_Index s = 0 ;

        switch (method)
        {

            case 0:
                // BFS using vector assign and reduce
                printf ("\nmethod 5: vector assign and reduce:\n") ;
                simple_tic (tic) ;
                OK (bfs5m (&v, A2, s)) ;
                break ;

            case 1:
                // BFS using vector assign and reduce
                printf ("\nmethod 5: same but check each result\n") ;
                simple_tic (tic) ;
                OK (bfs5m_check (&v, A2, s)) ;
                break ;

            case 2:
                // BFS using unary operator
                printf ("\nmethod 6: apply unary operator\n") ;
                simple_tic (tic) ;
                OK (bfs6 (&v, A2, s)) ;
                break ;

            case 3:
                // BFS using unary operator
                printf ("\nmethod 6: same but check each result\n") ;
                simple_tic (tic) ;
                OK (bfs6_check (&v, A2, s)) ;
                break ;

            default:
                CHECK (false, GrB_INVALID_VALUE) ;
                break ;
        }

        //----------------------------------------------------------------------
        // report the results
        //----------------------------------------------------------------------

        t = simple_toc (tic) ;
        printf ("BFS time in seconds: %14.6f\n", t) ;

        GrB_Index nreachable = 0 ;

        OK (GrB_Vector_new (&is_reachable, GrB_BOOL, n)) ;
        OK (GrB_apply (is_reachable, NULL, NULL, GrB_IDENTITY_BOOL, v, NULL)) ;
        OK (GrB_reduce (&nreachable, NULL, GxB_PLUS_INT32_MONOID,
            is_reachable, NULL)) ;
        OK (GrB_free (&is_reachable)) ;
        // OK (GrB_Vector_nvals (&nreachable, v)) ;
        printf ("nodes reachable from node %.16g: %.16g out of %.16g\n",
            (double) s, (double) nreachable, (double) n) ;

//      // note the typecast to int32_t
//      // using a predefined monoid instead, GrB_MAX_INT32_MONOID.
//      OK (GrB_Monoid_new (&max_monoid, GrB_MAX_INT32, (int32_t) INT32_MIN)) ;

        // find the max BFS level
        int64_t nlevels = -1 ;
        OK (GrB_reduce (&nlevels, NULL, GxB_MAX_INT32_MONOID, v, NULL)) ;
        printf ("max BFS level: %.16g\n", (double) nlevels) ;

        fprintf (stderr, "nodes reached: %.16g of %.16g levels: %.16g "
            "time: %12.6f seconds\n", (double) nreachable, (double) n,
                (double) nlevels, t) ;

        OK (GrB_free (&v)) ;
    }

    // free all workspace, including A, v, and max_monoid if allocated
    FREE_ALL ;

    //--------------------------------------------------------------------------
    // now break something on purpose and report the error:
    //--------------------------------------------------------------------------

    if (n == 4)
    {
        // this fails because the compiler selects the GrB_Monoid_new_INT32
        // function (clang 8.0 on MacOSX, at least), since false is merely the
        // constant "0".
        GrB_Monoid Lor ;
        info = GrB_Monoid_new (&Lor, GrB_LOR, false) ;        
        printf ("\n------------------- this fails:\n%s\n", GrB_error ( )) ;
        GrB_free (&Lor) ;

        // this selects the correct GrB_Monoid_new_BOOL function
        info = GrB_Monoid_new (&Lor, GrB_LOR, (bool) false) ;        
        printf ("\n------------------- this is OK: %d (should be"
            " GrB_SUCCESS = %d)\n", info, GrB_SUCCESS) ;
        GrB_free (&Lor) ;
    }

    fprintf (stderr, "\n") ;
    GrB_finalize ( ) ;
}

