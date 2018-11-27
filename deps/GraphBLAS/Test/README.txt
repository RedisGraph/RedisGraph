SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2018, All Rights Reserved.
http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

GraphBLAS/Test:

This folder includes a MATLAB interface that tests each GraphBLAS function.  It
is meant for testing and development only, not for general usage.  The
functions are efficient and robust.  They are not well-suited for general use
because the interface is not designed to be simple, clean, elegant, and
well-documented.  Such an interface would interfere with the purpose of these
functions, which is to compute a result in GraphBLAS (via a GB_mex_*
mexFunction) and then to compare its results with the GB_spec_*.m MATLAB mimic
function.  None of the GB_mex_*.c mexFunctions have corresponding GB_mex_*.m
files to provide 'help GB_mex...' documentation.

Requirements:  the mex command must use a C compiler supporting ANSI C11.

To run the tests you must first compile "spok":

    cd spok
    spok_install

Next, in this directory, to compile and run all the tests (about 10 minutes):

    gbmake ; testall

Longer tests can be done as well (this can take a whole day):

    testall (1)

To run with malloc debugging (this will be slower):

    debug_on
    gbmake ; testall

To turn off malloc debugging:

    debug_off

Malloc debugging is very extensive.  When enabled, the GraphBLAS wrapper
functions for malloc/calloc/realloc/free (mxMalloc/etc in this MATLAB
interface) decrement a counter when they are been successful.  When this
reaches zero, they pretend to fail, and thus allow the out-of-memory error
handling in GraphBLAS to be tested.  If the function fails, the counter is
reset, and the test is done again.  The initial state of this counter is
increased until the function succeeds.  During this entire process, a count is
kept of malloc'd blocks, and an error is reported if a leak is found.
GraphBLAS will be very slow with malloc debugging enabled.  It is only
done through the MATLAB interface and has no effect when GraphBLAS is
used through a C program (as in the Demo/ programs).

To enable further debugging tests, see the comments in Source/GB.h.

--------------------------------------------------------------------------------
Files and folders in GraphBLAS/Test:
--------------------------------------------------------------------------------

GB_spec_*.m     'spec' scripts.  These are very concise scripts, yet they
                perfectly match the GraphBLAS spec.  They are very slow since
                they rely on MATLAB dense matrices, since MATLAB only supports
                sparse logical, double, and double complex matrices.

GB_mex_*.c      mexFunction interfaces to GraphBLAS functions.  Some of these
                are interfaces to internal functions in SuiteSparse:GraphBLAS
                that are not in the GraphBLAS spec.  All of the functions rely
                on the specific details of the SuiteSparse:GraphBLAS
                implementation of the GraphBLAS specification, and thus they
                are not suitable as general interfaces to any GraphBLAS
                implementation.

GB_mx_*.c       helper functions for the mexFunctions
GB_mex.h        include file for the mexFunctions
Template/*.c    a template file for building two mexFunctions:
                GB_mex_Matrix_build and GB_mex_Vector_build.

gbmake.m        compiles the MATLAB interface to GraphBLAS

debug_off.m     turns off malloc debugging
debug_on.m      turns on malloc debugging; the interface will be *very* slow,
                since the problem is solved many times, with slowing increasing
                allotment of malloc's, until it succeeds.

stat.m          report on whether malloc debugging is enabled, and the current
                test coverage (see ../Tcov).

testall.m       a complete set of tests for GraphBLAS
testperf.m      performance tests for GraphBLAS
testperf_diary.txt  diary of testperf.m on a MacBookPro, 2.8 GHz Intel Core
                    i7, 16 GB Ram, OSX 10.11.6, clang 8.0.0, MATLAB R2017A.

spok            an acronym for "sparse OK"; checks if a MATLAB sparse matrix
                is valid

README.txt      this file

Contents.m      list of m-files in this folder

GB_define.m     create #defines for GraphBLAS.h

GB_user_*.m     test user-defined complex type and operators

Makefile        only 'make clean' and 'make distclean'

For all other *.m files, see Contents.m for a description.
