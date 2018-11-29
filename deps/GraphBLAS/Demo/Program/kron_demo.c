//------------------------------------------------------------------------------
// GraphBLAS/Demo/Program/kron_demo.c: Kronkecker product
//------------------------------------------------------------------------------

// SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
// http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

//------------------------------------------------------------------------------

// Reads two graphs from two files and computes their Kronecker product,
// C = kron (A,B), writing the result to a file.
//
//  kron_demo A.tsv B.tsv C.tsv
//
// Where A.tsv and B.tsv are two tab- or space-delimited triplet files with
// 1-based indices.  Each line has the form:
//
//  i j x
//
// where A(i,j)=x is performed by GrB_Matrix_build, to construct the matrix.
// The dimensions of A and B are assumed to be the largest row and column
// indices that appear in the files.  The file C.tsv is the filename of the
// output file for C=kron(A,B), also with 1-based indices.

#include "demos.h"

// macro used by OK(...) to free workspace if an error occurs
#define FREE_ALL                \
    GrB_free (&A) ;             \
    GrB_free (&B) ;             \
    GrB_free (&C) ;             \
    if (Afile != NULL) fclose (Afile) ; \
    if (Bfile != NULL) fclose (Bfile) ; \
    if (Cfile != NULL) fclose (Cfile) ; \
    if (I != NULL) free (I) ;   \
    if (J != NULL) free (J) ;   \
    if (X != NULL) free (X) ;   \
    GrB_finalize ( ) ;

int main (int argc, char **argv)
{
    //--------------------------------------------------------------------------
    // check inputs
    //--------------------------------------------------------------------------

    GrB_Matrix A = NULL, B = NULL, C = NULL ;
    GrB_Index *I = NULL, *J = NULL ;
    FILE *Afile = NULL, *Bfile = NULL, *Cfile = NULL ;
    double *X = NULL ;
    GrB_Info info ;
    double tic [2], t ;

    OK (GrB_init (GrB_NONBLOCKING)) ;

    // printf ("argc %d\n", argc) ;
    if (argc != 4)
    {
        FREE_ALL ;
        fprintf (stderr, "usage: kron_demo A.tsv B.tsv C.tsv\n") ;
        exit (1) ;
    }

    Afile = fopen (argv [1], "r") ;
    Bfile = fopen (argv [2], "r") ;
    Cfile = fopen (argv [3], "w") ;
    if (Afile == NULL || Bfile == NULL || Cfile == NULL)
    {
        FREE_ALL ;
        fprintf (stderr, "unable to read input files or create output file\n") ;
        exit (1) ;
    }

    //--------------------------------------------------------------------------
    // get A and B from input files
    //--------------------------------------------------------------------------

    // this would be faster and take less memory if GraphBLAS had a built-in
    // read-from-file operation
    OK (read_matrix (&A, Afile, false, false, true, false, false)) ;
    OK (read_matrix (&B, Bfile, false, false, true, false, false)) ;

    fclose (Afile) ;
    fclose (Bfile) ;
    Afile = NULL ;
    Bfile = NULL ;

    GrB_Index anrows, ancols, bnrows, bncols, anvals, bnvals ;
    OK (GrB_Matrix_nrows (&anrows, A)) ;
    OK (GrB_Matrix_ncols (&ancols, A)) ;
    OK (GrB_Matrix_nvals (&anvals, A)) ;
    OK (GrB_Matrix_nrows (&bnrows, B)) ;
    OK (GrB_Matrix_ncols (&bncols, B)) ;
    OK (GrB_Matrix_nvals (&bnvals, B)) ;

    //--------------------------------------------------------------------------
    // C = kron (A,B)
    //--------------------------------------------------------------------------

    OK (GrB_Matrix_new (&C, GrB_FP64, anrows * bnrows, ancols * bncols)) ;

    simple_tic (tic) ;
    OK (GxB_kron (C, NULL, NULL, GrB_TIMES_FP64, A, B, NULL)) ;
    t = simple_toc (tic) ;

    OK (GrB_free (&A)) ;
    OK (GrB_free (&B)) ;

    //--------------------------------------------------------------------------
    // report results
    //--------------------------------------------------------------------------

    GrB_Index cnrows, cncols, cnvals ;
    OK (GrB_Matrix_nrows (&cnrows, C)) ;
    OK (GrB_Matrix_ncols (&cncols, C)) ;
    OK (GrB_Matrix_nvals (&cnvals, C)) ;

    // note that integers of type GrB_Index should be printed with the
    // %PRIu64 format.

    fprintf (stderr, "GraphBLAS GxB_kron:\n"
    "A: %" PRIu64 "-by-%" PRIu64 ", %" PRIu64 " entries.\n"
    "B: %" PRIu64 "-by-%" PRIu64 ", %" PRIu64 " entries.\n"
    "C: %" PRIu64 "-by-%" PRIu64 ", %" PRIu64 " entries.\n"
    "time: %g seconds, rate: nval(C)/t = %g million/sec\n",
    anrows, ancols, anvals,
    bnrows, bncols, bnvals,
    cnrows, cncols, cnvals,
    t, 1e-6*((double) cnvals) / t) ;

    //--------------------------------------------------------------------------
    // write C to the output file
    //--------------------------------------------------------------------------

    // this would be faster and take less memory if GraphBLAS had a built-in
    // write-to-file operation

    I = (GrB_Index *) malloc ((cnvals+1) * sizeof (GrB_Index)) ;
    J = (GrB_Index *) malloc ((cnvals+1) * sizeof (GrB_Index)) ;
    X = (double    *) malloc ((cnvals+1) * sizeof (double   )) ;
    if (I == NULL || J == NULL || X == NULL)
    {
        fprintf (stderr, "out of memory\n") ;
        FREE_ALL ;
        exit (1) ;
    }

    OK (GrB_Matrix_extractTuples (I, J, X, &cnvals, C)) ;

    for (int64_t k = 0 ; k < cnvals ; k++)
    {
        fprintf (Cfile, "%" PRIu64 "\t%" PRIu64 "\t%.17g\n", 1 + I [k], 1 + J [k], X [k]) ;
    }

    FREE_ALL ;
    return (0) ;
}

