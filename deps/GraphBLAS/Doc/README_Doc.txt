--------------------------------------------------------------------------------
SuiteSparse:GraphBLAS/Doc folder
--------------------------------------------------------------------------------

This folder contains the following files:

    CONTRIBUTOR-LICENSE.txt     how to contribute to GraphBLAS
    ChangeLog                   changes in GraphBLAS
    GraphBLAS_API_C.pdf         the GraphBLAS C API Specification
    GraphBLAS_API_version.tex   the version of the C API that this version
                                of SuiteSparse:GraphBLAS conforms to.
    GraphBLAS_UserGuide.pdf     the SuiteSparse:GraphBLAS User Guide 

    GraphBLAS_UserGuide.tex     LaTeX source of the User Guide
    GraphBLAS_UserGuide.bib
    Makefile                    to create the User Gude

    GraphBLAS_version.tex       this version of SuiteSparse:GraphBLAS
    License.txt                 the license: Apache 2.0
    README_Doc.txt              this file

Papers on SuiteSparse:GraphBLAS.  See the User Guide for the full citations.

    toms_graphblas.pdf          2019 ACM TOMS paper on SuiteSparse:GraphBLAS
    CSC20_OpenMP_GraphBLAS.pdf  "Parallel GraphBLAS with OpenMP"
    HPEC19.pdf                  "Write Quick, Run Fast: Sparse DNN ..."
    Davis_HPEC18.pdf            "Graph algorithms via SuiteSparse:GraphBLAS..."
    lagraph-grapl19.pdf         "LAGraph: a community effort to collect graph
                                 algorithms built on top of the GraphBLAS"

Additional installation notes are below.

--------------------------------------------------------------------------------

SuiteSparse:GraphBLAS requires OpenMP for its internal parallelism.  It also
needs either POSIX pthreads or OpenMP to be thread-safe, for multithreaded user
applications.  The Mac has POSIX pthreads built-in, which works fine for
user-thread safety, but will not be enough to get internal parallelism in
GraphBLAS.  To install OpenMP on the Mac, try these instructions.

To use OpenMP in GraphBLAS on the Mac:

If you have the Intel compiler and OpenMP library, then use the following
in the GraphBLAS/ folder.  OpenMP will be found automatically:

    make CC=icc CXX=icc

Otherwise, you can use gcc-8 and the OpenMP library in the Mac HomeBrew,
as follows.

The following process was done on a MacBook Pro (Retina, 13-inch, Late 2013)
with macOS Mojavae 10.14.1, on Nov 26, 2018:

First, install Xcode 10.1 from the Apple App Store, and the Command Line Tools
for macOS 10.14 for Xcode 10.1 from https://developer.apple.com/download/more/

Next, install brew (https://brew.sh).
THen install OpenMP and its library using brew:

brew install open-mp
brew install libopmp

The latter reports the following Caveat:

    ==> Downloading https://homebrew.bintray.com/bottles/libomp-7.0.0.mojave.bottle.
    ######################################################################## 100.0%
    ==> Pouring libomp-7.0.0.mojave.bottle.tar.gz
    ==> Caveats
    On Apple Clang, you need to add several options to use OpenMP's front end
    instead of the standard driver option. This usually looks like
      -Xpreprocessor -fopenmp -lomp
     
    You might need to make sure the lib and include directories are discoverable
    if /usr/local is not searched:
     
      -L/usr/local/opt/libomp/lib -I/usr/local/opt/libomp/include
     
     or CMake, the following flags will cause the OpenMP::OpenMP_CXX target to
    be set up correctly:
      -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include" -DOpenMP_CXX_LIB_NAMES="omp" -DOpenMP_omp_LIBRARY=/usr/local/opt/libomp/lib/libomp.dylib
    ==> Summary
    /usr/local/Cellar/libomp/7.0.0: 12 files, 1.2MB


This installs gcc version 8.2 as well.

Use the cmake that comes with Xcode 10.1 (cmake version 3.4.1).  You could
also use

brew install cmake

to use cmake 3.13.0, but I haven't tried that.  That step would need to
remove the links in /usr/local/bin/ to the cmake in Xcode 10.1, and
replace them with the brew verions.

Then do the following, as instructed by the brew Caveat about libomp:

cd SuiteSparse/GraphBLAS/build
CC=gcc-8 cmake  -DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp -I/usr/local/opt/libomp/include" -DOpenMP_CXX_LIB_NAMES="omp" -DOpenMP_omp_LIBRARY=/usr/local/opt/libomp/lib/libomp.dylib ..

which should report that OpenMP will be used to synchronize user threads.
I got a warning that OpenMP_CXX_LIB_NAMES and OpenMP_omp_LIBRARY
were specified but not used.  GraphBLAS/CMakeLists.txt probably needs
to be modified to add these to the clang flags, but they are not needed
if compiling with gcc-8.

Then in GraphBLAS/build, do:

make

or for a faster build on a 4-core system:

make JOBS=4

To test it, go to SuiteSparse/GraphBLAS/  and type

make demo

check out the openmp_demo.out file to see if OpenMP was used.
