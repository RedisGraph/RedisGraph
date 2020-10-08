# SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

VERSION 3.2.0, Feb 20, 2020

SuiteSparse:GraphBLAS is an full implementation of the GraphBLAS standard,
which defines a set of sparse matrix operations on an extended algebra of
semirings using an almost unlimited variety of operators and types.  When
applied to sparse adjacency matrices, these algebraic operations are equivalent
to computations on graphs.  GraphBLAS provides a powerful and expressive
framework for creating graph algorithms based on the elegant mathematics of
sparse matrix operations on a semiring.

See the user guide in `Doc/GraphBLAS_UserGuide.pdf` for documentation on the
SuiteSparse implementation of GraphBLAS, and how to use it in your
applications.

See http://graphblas.org for more information on GraphBLAS, including the
GraphBLAS C API (also in `Doc/GraphBLAS_API_C.pdf`).  See
https://github.com/szarnyasg/graphblas-pointers for additional resources on
GraphBLAS.

QUICK START: To compile, run several demos, and install, do these commands in
this directory:

    make
    sudo make install

Please be patient; some files can take several minutes to compile.  Requires an
ANSI C11 compiler, so cmake will fail if your compiler is not C11 compliant.
See the User Guide PDF in Doc/ for directions on how to use another compiler.

For faster compilation, do this instead of just "make", which uses 32
parallel threads to compile the package:

    make JOBS=32

The output of the demo programs will be compared with their expected output.

To remove all compiled files:

    make clean

To compile the library without running the demos or installing it:

    make library

See the GraphBLAS/ subfolder for the MATLAB interface, which contains a
README.md file with further details.

--------------------------------------------------------------------------------
## Files and folders in this GraphBLAS directory:

CMakeLists.txt:  cmake instructions to compile GraphBLAS

Config:         version-dependent files used by CMake

Demo:           a set of demos on how to use GraphBLAS

Doc:            SuiteSparse:GraphBLAS User Guide and license

Extras:         parallel methods: triangle counting, k-truss, and a
                massively parallel (MPI) Kronecker product matrix generator.
                These are stand-along package that rely on GraphBLAS.  They
                are not compiled by the cmake script.  See Extras/README.txt
                for more details.

GraphBLAS:      the MATLAB interface.  This folder is called 'GraphBLAS' so
                that typing 'help graphblas' or 'doc graphblas' in the MATLAB
                Command Window can locate the Contents.m file.

Include:        user-accessible include file, GraphBLAS.h

Makefile:       to compile the SuiteSparse:GraphBLAS library and demos

README.md:      this file

Source:         source files of the SuiteSparse:GraphBLAS library.

Tcov:           test coverage, requires MATLAB

Test:           Extensive tests, not meant for general usage.  To compile
                SuiteSparse:GraphBLAS and test in MATLAB, go to this directory
                and type make;testall in MATLAB.

build:          build directory for CMake, initially empty

alternative:    an alternative to CMake; edit the alternative/Makefile and do
                "make" or "make run" in the 'alternative' directory.

--------------------------------------------------------------------------------

## SPEC:

This version fully conforms to the version 1.2.0 (May 18, 2018)
of the GraphBLAS C API Specification.  It includes several additional functions
and features as extensions to the spec.  These extensions are tagged with the
keyword SPEC: in the code and in the User Guide, and in the Include/GraphBLAS.h
file.  All functions, objects, and macros with the prefix GxB are extensions to
the spec.  Functions, objects, and macros with prefix GB must not be accessed
by user code.  They are for internal use in GraphBLAS only.


--------------------------------------------------------------------------------
## For Windows users:

This version is not compatible with Microsoft Visual Studio.  Use another
compiler, or use SuiteSparse:GraphBLAS v3.1.2 instead.  See the User Guide
for more details.

