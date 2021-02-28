//------------------------------------------------------------------------------
// kron.c: Kronkecker product using GraphBLAS
//------------------------------------------------------------------------------

// Timothy A. Davis, (c) 2018, All Rights Reserved.  License: Apache 2.0
// (same as GraphBLAS)

//------------------------------------------------------------------------------

// Reads two graphs from two files and computes their Kronecker product,
// as C = kron (A,B) in MATLAB, writing the result to a file.
//
//  kron A.tsv B.tsv C.tsv np pid btype
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
//
// The np, pid, and btype parameters are optional.
//
// The defaults for np and pid, if not present, are np=1 and pid=0.  np is the
// number of processors who are working together to create C, and pid is in the
// range 0 to np-1.  If present, the output filename C.tsv is prepended with
// "<pid>_", so C.tsv becomes 0_C.tsv, 1_C.tsv, etc.
//
// If not present, the default for btype is zero.  If btype=1 or 2, then A and
// B are modified to generate lots (btype=1) or some (btype=2) extra triangles.

#include "kron.h"

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // check command-line inputs
    //--------------------------------------------------------------------------

    if (! (argc == 4 || argc == 6 || argc == 7))
    {
        fprintf (stderr, "usage: kron A.tsv B.tsv C.tsv np pid btype\n") ;
        exit (1) ;
    }

    char *Afilename = argv [1] ;
    char *Bfilename = argv [2] ;
    char *Cfilename = argv [3] ;

    int np = 1, pid = 0 ;
    if (argc >= 6)
    {
        sscanf (argv [4], "%d", &np) ;
        sscanf (argv [5], "%d", &pid) ;
    }

    int btype = 0 ;
    if (argc == 7)
    {
        sscanf (argv [6], "%d", &btype) ;
    }

    //--------------------------------------------------------------------------
    // construct C = kron(A,B) or a submatrix of C
    //--------------------------------------------------------------------------

    return (kron_submatrix (Afilename, Bfilename, Cfilename, np, pid, btype)) ;
}

