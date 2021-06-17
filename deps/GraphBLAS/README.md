# SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
SPDX-License-Identifier: Apache-2.0

VERSION 4.0.1, Jan 4, 2021

SuiteSparse:GraphBLAS is complete implementation of the GraphBLAS standard,
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
https://github.com/GraphBLAS/GraphBLAS-Pointers
for additional resources on GraphBLAS.


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

## GraphBLAS C API Specification:

This version fully conforms to the version 1.3.0 (Sept 25, 2019)
of the GraphBLAS C API Specification.  It includes several additional functions
and features as extensions to the spec.

All functions, objects, and macros with the prefix GxB are extensions to
the spec.  Functions, objects, and macros with prefix GB must not be accessed
by user code.  They are for internal use in GraphBLAS only.

--------------------------------------------------------------------------------

## About NUMA systems

I have tested this package extensively on multicore single-socket systems, but
have not yet optimized it for multi-socket systems with a NUMA architecture.
That will be done in a future release.  If you publish benchmark comparisons
with this package, please state the SuiteSparse:GraphBLAS version, and a caveat
if appropriate.  If you see significant performance issues when going from a
single-socket to multi-socket system, I would like to hear from you so I can
look into it.  Contact me at davis@tamu.edu.

