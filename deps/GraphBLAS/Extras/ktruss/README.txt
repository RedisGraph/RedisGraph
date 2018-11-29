ktruss:  construct the k-truss of a graph, using MATLAB, pure C code (in
parallel using OpenMP, not using GraphBLAS), and GraphBLAS.

June 23, 2018

See ktruss.m for a description.  Files in this package:

    Makefile                    compile each program
    README.txt                  this file
    allktruss.c                 construct all k-trusses (pure C)
    allktruss.m                 construct all k-trusses (in MATLAB)
    allktruss_graphblas.c       construct all k-trusses (with GraphBLAS)
    allktruss_graphblas_main.c  main program for allktruss_graphblas.c
    allktruss_main.c            main program for allktruss.c
    allktruss_mex.c             MATLAB mexFunction interface for allktruss.c
    bcsstk01                    small input test matrix
    kgo                         run all tests
    kgo.m                       run tests in MATLAB
    kgo1                        run just two matrices
    kmake.m                     compile the mexFunctions
    krun                        run one matrix
    ktruss.c                    construct a ktruss (pure C)
    ktruss.m                    construct a ktruss (in MATLAB)
    ktruss_def.h                include file for allktruss and ktruss
    ktruss_graphblas.c          construct a ktruss (in GraphBLAS)
    ktruss_graphblas_def.h      include file for *graphblas.c
    ktruss_graphblas_main.c     main program for ktruss_graphblas.c
    ktruss_main.c               main program for ktruss.c
    ktruss_mex.c                MATLAB mexFunction interface for ktruss.c
    ktruss_ntriangles.c         count # of triangles
    ktruss_read.c               read a matrix from a file

    allktruss_grb_results.m     allktruss GraphBLAS results
    allktruss_results.m         allktruss.c results
    ktruss_grb_results.m        ktruss GraphBLAS results
    ktruss_results.m            ktruss.c results

    chol2_results.txt           raw results on cholesky.cse.tamu.edu, IBM Minsky
    filetrim.m                  trim filename
    k1                          run one matrix (friendster)

    kres14.m                    analyze results (except for last matrix)
    kres.m                      analyze results
    ksub                        run subset of GraphChallenge for HPEC18

To compile the package:

    Edit the Makefile and pick your compiler.
    Install SuiteSparse:GraphBLAS.  Version 2.0.1 was used.
    Edit ktruss_main.c to select the max # of threads.
    Do:

        make
        ./kgo1
        ./kgo

For the kgo* scipts, you will need the matrices from the GraphChallenge
data sets in ~/GraphChallenge, gzipped to save space.  You will also need
some matrices from the SuiteSparse collection (the ~/GraphChallenge/ssget/
folder).

