//------------------------------------------------------------------------------
// kron.c: Kronkecker product using GraphBLAS
//------------------------------------------------------------------------------

// Timothy A. Davis, (c) 2018, All Rights Reserved.  License: Apache 2.0
// (same as GraphBLAS)

//------------------------------------------------------------------------------

// Reads two graphs from two files and computes their Kronecker product,
// as C = kron (A,B) in MATLAB, writing the result to a file.
//
//  mpirun -np 4 ./kron_mpi A.tsv B.tsv C.tsv btype
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
// The btype parameter is optional.
//
// If not present, the default for btype is zero.  If btype=1 or 2, then A and
// B are modified to generate lots (btype=1) or some (btype=2) extra triangles.
//
// If np is the number of MPI processors who are working together to create C,
// and pid is in the range 0 to np-1, then the output filename C.tsv is
// prepended with "<pid>_", so C.tsv becomes 0_C.tsv, 1_C.tsv, etc.

#include "kron.h"
#include <mpi.h>

// call an MPI function and abort if an error occurs
#define OK(mpi_operation)                                               \
{                                                                       \
    int ierr = mpi_operation ;                                          \
    if (ierr != MPI_SUCCESS)                                            \
    {                                                                   \
        fprintf (stderr, "MPI error (%s line %d, pid %d of %d): %d\n",  \
            __FILE__, __LINE__, pid, np, ierr) ;                        \
        MPI_Abort (MPI_COMM_WORLD, ierr) ;                              \
    }                                                                   \
}

int main (int argc, char **argv)
{

    //--------------------------------------------------------------------------
    // start MPI
    //--------------------------------------------------------------------------

    int np = 1, pid = 0 ;
    OK (MPI_Init (&argc, &argv)) ;                  // start MPI
    OK (MPI_Comm_size (MPI_COMM_WORLD, &np)) ;      // get # of MPI processes
    OK (MPI_Comm_rank (MPI_COMM_WORLD, &pid)) ;     // get this process id

    //--------------------------------------------------------------------------
    // check command-line inputs
    //--------------------------------------------------------------------------

    if (! (argc == 4 || argc == 5))
    {
        fprintf (stderr, "usage: kron_mpi A.tsv B.tsv C.tsv np pid btype\n") ;
        exit (1) ;
    }
    char *Afilename = argv [1] ;
    char *Bfilename = argv [2] ;
    char *Cfilename = argv [3] ;
    int btype = 0 ;
    if (argc == 7)
    {
        sscanf (argv [4], "%d", &btype) ;
    }

    //--------------------------------------------------------------------------
    // construct C = kron(A,B) or a submatrix of C
    //--------------------------------------------------------------------------

    int err = kron_submatrix (Afilename, Bfilename, Cfilename, np, pid, btype) ;

    //--------------------------------------------------------------------------
    // wrap up
    //--------------------------------------------------------------------------

    if (err)
    {
        fprintf (stderr, "kron_submatrix failed (%s line %d, pid %d of %d)\n",
            __FILE__, __LINE__, pid, np) ;
        MPI_Abort (MPI_COMM_WORLD, MPI_ERR_OTHER) ;
    }

    MPI_Finalize ( ) ;
}

