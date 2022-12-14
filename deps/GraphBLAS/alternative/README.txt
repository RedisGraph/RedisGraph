SuiteSparse/GraphBLAS/alternative/README.txt

SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
SPDX-License-Identifier: Apache-2.0

CMake is prefered for compiling SuiteSparse/GraphBLAS, but this folder provides
a simple alternative, if you don't have CMake or if you prefer a simple
Makefile build process.  The Makefile will typically require modifications
before it will work on your system.  It assumes you have the GNU compiler
(gcc), but this can easily be changed by editting the Makefile.  See the ifeq
(UNAME...) section in the Makefile to customize the settings for Linux or the
Mac.

A C++ compiler works but is not recommended for use in production,
particularly if SuiteSparse:GraphBLAS is used as -lgraphblas.
SuiteSparse:GraphBLAS is written in C, not C++.  However, it compiles just
fine with a C++ compiler, since it is written so that it uses the
intersection of the two languages.  This is helpful for two cases: (1)
SuiteSparse:GraphBLAS does a lot of typecasting, and C++ is very strict with
this.  Using C++ allows for an extra-careful 'lint' checking.  (2) The end-
user may wish to construct a pure C++ application, and embed a C++-compiled
GraphBLAS inside.  CMake cannot be used to compile a *.c code with a C++
compiler (it complains), and thus this option is only available in this
alternative/Makefile.  The GCC C++ 5.4 compiler fails; version 7.5 is
sufficient.

Files:

    Makefile
    README.txt
    altdemo

to compile the dynamic library:       make
to compile the static library:        make static
to compile in parallel with 4 cores:  make -j4
to install in /usr/local/*:           sudo make install
to cleanup:                           make distclean
to compile and run the demos:         make demo

