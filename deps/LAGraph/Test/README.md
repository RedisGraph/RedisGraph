# LAGraph/Test
This folder contains a collection of independent test programs.
Currently there is only one:

    MatrixMarket:  tests LAGraph_mmread and LAGraph_mmwrite

The top-level Makefile in this folder should cd to each
Test/xxx folder, one at a time, to compile and run the test.

The MatrixMarket folder contains a simple CMake script,
a Makefile, and build/ folder, and a single main program,
mmtest.c.  Additional tests can be added here by copying
this folder and creating an new test.

LAGraph and GraphBLAS must first be compiled (and installed if
need be) before compiling and running these tests.

Authors: (... list them here)

