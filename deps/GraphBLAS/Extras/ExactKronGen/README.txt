kron: Kronkecker product using GraphBLAS
Timothy A. Davis, (c) 2018, All Rights Reserved.  License: Apache 2.0
(same as GraphBLAS)

ExactKronGen, July 19, 2018

First install GraphBLAS in a system-wide location (so that -lgraphblas and
#include<GraphBLAS.h> work).  Then, to compile and test without MPI:

    make

To compile and test with MPI:

    make mpi

The Makefile assumes -lgraphblas exists, and #include<GraphBLAS.h> works.  Be
sure to use gcc 4.9 or later, or icc version 18 or later.  See the
GraphBLAS/CMakeLists.txt for details.  For example:

    module load beta-gcc-5.2.0
    make CC=gcc
    make CC=gcc mpi

See kron.c and kron_mpi.c for how to use the kron program from
the command line.

On the MIT Lincoln Lab systems, the gokron and gokron_mpi
scripts can be used, which sets the library path to where
GraphBLAS is installed:

    ./gokron
    ./gokron_mpi

Files:

    README.txt          this file

    Makefile            to compile and test kron and kron_mpi
    a.tsv               sample input
    b.tsv               sample input
    corig.tsv           sample output

    kron_mpi.c          main program, with MPI
    kron.c              main program, no MPI
    kron.h              include file
    kron_submatrix.c    create submatrix of C=kron(A,B)
    kron_test.m         test kron.c in MATLAB
    read_tuples.c       read tuples from a file
    simple_timer.c      a simple timer
    simple_timer.h      include file for simple_timer.c

    system specific, only works on MIT system:
    data                symbolic link to GraphChallenge files
    gokron              run on MIT system, 4 submatrices, no MPI
    gokron_mpi          run on MIT system, 4 submatrices, with MPI

    gokron_output.txt       output of gokron
    gokron_mpi_output.txt   output of gokron_mpi

