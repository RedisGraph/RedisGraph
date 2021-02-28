GraphBLAS extras:

These programs are not necessarily ported to any system.  You may need to
modify the Makefiles or the scripts the run the tests.

    ExactKronGen:
    
        Massively-parallel Kronecker product computation.This package is a
        simple program for constructing a huge Kronecker product in parallel.
        It includes a top-level MPI-based program. It also includes a simple
        function (kron_submatrix) that can be used inside a larger MPI
        application, to run on a single MPI process or also without MPI at all.
        The function computes a submatrix of a larger Kronecker product and
        writes its submatrix to a uniquely-named file. Concatenating all such
        files together gives the resulting Kronecker product. The package uses
        GxB_kron in SuiteSparse:GraphBLAS.

    tri: triangle counting in OpenMP.  See also GraphBLAS/Demo/tricount.c

    ktruss: K-truss computation in OpenMP and GraphBLAS

