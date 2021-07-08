# SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.

SPDX-License-Identifier: Apache-2.0

For the GraphBLAS/GraphBLAS Octave/MATLAB interface *only*:
SPDX-License-Identifier: GPL-3.0-or-later
(see below for a discussion of the licensing of this package).

VERSION 5.1.4, July 6, 2021

SuiteSparse:GraphBLAS is a complete implementation of the GraphBLAS standard,
which defines a set of sparse matrix operations on an extended algebra of
semirings using an almost unlimited variety of operators and types.  When
applied to sparse adjacency matrices, these algebraic operations are equivalent
to computations on graphs.  GraphBLAS provides a powerful and expressive
framework for creating graph algorithms based on the elegant mathematics of
sparse matrix operations on a semiring.

SuiteSparse:GraphBLAS is used heavily in production.  It appears as the
underlying graph engine in the RedisGraph database by Redis Labs, and as the
built-in sparse matrix multiply in MATLAB R2021a, where `C=A*B` is now up to
30x faster than in prior versions of MATLAB (on my 20-core NVIDIA DGX Station).

The development of this package is supported by NVIDIA (including the donation
of the 20-core DGX Station), Redis Labs, MIT Lincoln Lab, Intel, and IBM.

See the user guide in `Doc/GraphBLAS_UserGuide.pdf` for documentation on the
SuiteSparse implementation of GraphBLAS, and how to use it in your
applications.

See http://graphblas.org for more information on GraphBLAS, including the
GraphBLAS C API (also in `Doc/GraphBLAS_API_C.pdf`).  See
https://github.com/GraphBLAS/GraphBLAS-Pointers
for additional resources on GraphBLAS.

QUICK START: To compile and install, do these commands in this directory:

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

To compile the demos:

    make all

See the GraphBLAS/ subfolder for the Octave/MATLAB interface, which contains a
README.md file with further details.

--------------------------------------------------------------------------------
## Files and folders in this GraphBLAS directory:

CMakeLists.txt:  cmake instructions to compile GraphBLAS

Config:         version-dependent files used by CMake

Demo:           a set of demos on how to use GraphBLAS

Doc:            SuiteSparse:GraphBLAS User Guide and license

GraphBLAS:      the @GrB Octave/MATLAB interface, including its test suite and
                demos.  This folder is called 'GraphBLAS' so that typing 'help
                graphblas' or 'doc graphblas' in the Octave or MATLAB Command
                Window can locate the Contents.m file.  Note that this folder
                and all its contents are under the GNU GPLv3 (or later), not
                Apache-2.0.  All other components of GraphBLAS (in particular,
                all code in libgraphblas.so, and the entire CUDA folder) are
                licensed as Apache-2.0.

Include:        user-accessible include file, GraphBLAS.h

Makefile:       to compile the SuiteSparse:GraphBLAS library and demos

README.md:      this file

Source:         source files of the SuiteSparse:GraphBLAS library.

Tcov:           test coverage, requires Octave or MATLAB

Test:           Extensive tests, not meant for general usage.  To compile and
                run, go to this directory and type make;testall in Octave or
                MATLAB.  Requires Octave or MATLAB

build:          build directory for CMake, initially empty

alternative:    an alternative to CMake; edit the alternative/Makefile and do
                "make" or "make run" in the 'alternative' directory.

CUDA:           GPU interface, a work in progress.  This is being developed in
                collaboration with Joe Eaton and others at NVIDIA, with
                support from NVIDIA.  It appears in this release but the CUDA
                folder is a draft that isn't ready to use yet.

--------------------------------------------------------------------------------

## GraphBLAS C API Specification:

This version fully conforms to the version 1.3.0 (Sept 25, 2019)
of the GraphBLAS C API Specification.  It includes several additional functions
and features as extensions to the spec.

All functions, objects, and macros with the prefix GxB are extensions to
the spec.  Functions, objects, and macros with prefix GB must not be accessed
by user code.  They are for internal use in GraphBLAS only.

--------------------------------------------------------------------------------

## About Benchmarking

Do not use the demos in GraphBLAS/Demos for benchmarking or in production.
Those are simple methods for illustration only, and can be slow.  Use LAGraph
for benchmarking and production uses.

I have tested this package extensively on multicore single-socket systems, but
have not yet optimized it for multi-socket systems with a NUMA architecture.
That will be done in a future release.  If you publish benchmarks
with this package, please state the SuiteSparse:GraphBLAS version, and a caveat
if appropriate.  If you see significant performance issues when going from a
single-socket to multi-socket system, I would like to hear from you so I can
look into it.

Contact me at davis@tamu.edu for any questions about benchmarking
SuiteSparse:GraphBLAS and LAGraph.

--------------------------------------------------------------------------------

## Licensing and supporting SuiteSparse:GraphBLAS

SuiteSparse:GraphBLAS is released primarily under the Apache-2.0 license,
because of how the project is supported by many organizations (NVIDIA, Redis
Labs, MIT Lincoln Lab, Intel, and IBM), primarily through gifts to the Texas
A&M Foundation.  Because of this support, and to facilitate the wide-spread use
of GraphBLAS, the decision was made to give this library a permissive
open-source license (Apache-2.0).  Currently all source code required to create
the C-callable library libgraphblas.so is licensed with Apache-2.0, and there
are no plans to change this.

However, just because this code is free to use doesn't make it zero-cost to
create.  If you are using GraphBLAS in a commercial closed-source product and
are not supporting its development, please consider supporting this project
to ensure that it will continue to be developed and enhanced in the future.

The Octave/MATLAB interface is now licensed under the GNU GPLv3.0 (or later)
license, so if any code from the GraphBLAS/GraphBLAS folder is needed in a
commercial closed-source application, you will either need to license your
commercial application as GNU GPLv3.0, or you can ask me for a non-GNU and
non-Apache license.  This change to the licensing of the Octave/MATLAB
interface is retroactive to all versions of GraphBLAS (back to v1.0 in 2017); I
have not updated the github releases with the text to change this license,
because this would be disruptive to version management systems.  However, if
you see an "Apache-2.0" stamp on any file in the GraphBLAS/GraphBLAS folder or
its subfolders, ignore it and assume GPLv3.

This licensing change has no impact at all on the use of GraphBLAS in Octave,
since Octave itself is already under the GNU GPLv3.0 license.

To support the development of GraphBLAS, contact the author (davis@tamu.edu) or
the Texas A&M Foundation (True Brown, tbrown@txamfoundation.com; or Kevin
McGinnis, kmcginnis@txamfoundation.com) for details.

SuiteSparse:GraphBLAS, is copyrighted by Timothy A. Davis, (c) 2017-2021, All
Rights Reserved.  davis@tamu.edu.  Contact me if you need a non-GNU license.

