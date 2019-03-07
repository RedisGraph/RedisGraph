//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/complex_demo.c: demo for user-defined complex type
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// This demo illustrates the creation and use of a user-defined type for double
// complex matrices and vectors.  Run the output of this program in MATLAB
// to check the results.

#include "demos.h"

//------------------------------------------------------------------------------
// print a complex matrix
//------------------------------------------------------------------------------

// when printed, 1 is added to all row indices so the results can be
// checked in MATLAB

void print_complex_matrix (GrB_Matrix A, char *name)
{
    GrB_Index nrows, ncols, nentries ;

    GrB_Matrix_nvals (&nentries, A) ;
    GrB_Matrix_nrows (&nrows, A) ;
    GrB_Matrix_ncols (&ncols, A) ;

    printf (
        "\n%% GraphBLAS matrix %s: nrows: %.16g ncols %.16g entries: %.16g\n",
        name, (double) nrows, (double) ncols, (double) nentries) ;

    GrB_Index *I = malloc (MAX (nentries,1) * sizeof (GrB_Index)) ;
    GrB_Index *J = malloc (MAX (nentries,1) * sizeof (GrB_Index)) ;
    double complex *X = malloc (MAX (nentries,1) * sizeof (double complex)) ;

    GrB_Matrix_extractTuples_UDT (I, J, X, &nentries, A) ;

    printf ("%s = sparse (%.16g,%.16g) ;\n", name,
        (double) nrows, (double) ncols) ;
    for (int64_t k = 0 ; k < nentries ; k++)
    {
        printf ("    %s (%.16g,%.16g) =  (%20.16g) + (%20.16g)*1i ;\n",
            name, (double) (1 + I [k]), (double) (1 + J [k]),
            creal (X [k]), cimag (X [k])) ;
    }
    printf ("%s\n", name) ;

    free (I) ;
    free (J) ;
    free (X) ;
}

//------------------------------------------------------------------------------
// C = A*B for complex matrices
//------------------------------------------------------------------------------

int main (int argc, char **argv)
{
    GrB_Index m = 3, k = 5, n = 4 ;
    GrB_Matrix A, B, C ;

    // initialize GraphBLAS and create the user-defined Complex type
    GrB_init (GrB_NONBLOCKING) ;
    fprintf (stderr, "complex_demo:\n") ;
    Complex_init ( ) ;

    // generate random matrices A and B
    simple_rand_seed (1) ;
    random_matrix (&A, false, false, m, k, 6, 0, true) ;
    random_matrix (&B, false, false, k, n, 8, 0, true) ;

    GxB_fprint (A, GxB_SHORT, stderr) ;
    GxB_fprint (B, GxB_SHORT, stderr) ;

    // C = A*B
    GrB_Matrix_new (&C, Complex, m, n) ;
    GrB_mxm (C, NULL, NULL, Complex_plus_times, A, B, NULL) ;

    GxB_fprint (C, GxB_SHORT, stderr) ;

    // print the results
    printf ("\n%% run this output of this program as a script in MATLAB:\n") ;
    print_complex_matrix (A, "A") ;
    print_complex_matrix (B, "B") ;
    print_complex_matrix (C, "C") ;

    printf ("E = A*B\n") ;
    printf ("err = norm (C-E,1)\n") ;
    printf ("assert (err < 1e-12)\n") ;

    // free all matrices
    GrB_free (&A) ;
    GrB_free (&B) ;
    GrB_free (&C) ;

    // free the Complex types, operators, monoids, and semiring
    Complex_finalize ( ) ;

    // finalize GraphBLAS
    GrB_finalize ( ) ;
}

