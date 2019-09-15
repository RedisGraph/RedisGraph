//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/import_demo.c: test import/export
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Read a graph from a file and test import/export. Usage:
//
//  mis_demo < infile

#include "demos.h"

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                \
    GrB_free (&A) ;             \

int main (int argc, char **argv)
{
    GrB_Matrix A = NULL ;
    GrB_Info info ;
    OK (GrB_init (GrB_NONBLOCKING)) ;
    fprintf (stderr, "import_demo:\n") ;

    //--------------------------------------------------------------------------
    // get a matrix
    //--------------------------------------------------------------------------

    // usage:  ./main  < file
    //         ./main 0 dump < file
    //         ./main 1 dump < file
    //
    // default is 0-based, for the matrices in the Matrix/ folder

    bool one_based = false ;
    bool dump = false ;
    if (argc > 1) one_based = strtol (argv [1], NULL, 0) ;
    if (argc > 2) dump      = strtol (argv [2], NULL, 0) ;

    OK (read_matrix (&A, stdin, false, false, one_based, false, false)) ;

    for (int hyper = 0 ; hyper <= 1 ; hyper++)
    {
        for (int csc = 0 ; csc <= 1 ; csc++)
        {
            double h = hyper ? GxB_ALWAYS_HYPER : GxB_NEVER_HYPER ;
            GxB_Format_Value f = csc ? GxB_BY_COL : GxB_BY_ROW ;

            printf ("\n######### input A: hyper %d csc %d\n", hyper, csc) ;

            for (int format = 0 ; format <= 3 ; format++)
            {

                OK (GxB_set (A, GxB_HYPER, h)) ;
                OK (GxB_set (A, GxB_FORMAT, f)) ;
                OK (import_test (&A, format, dump)) ;
            }
        }
    }

    FREE_ALL ;

    OK (GrB_finalize ( )) ;
}

