# LAGraph
This is a library plus a test harness for collecting algorithms that use the
GraphBLAS.  It contains the following files and folders:

    CMakeLists.txt: a CMake script for compiling

    Doc: documentation, including the results of doxygen

    Doxyfile: to build the doxygen documentation, do "make dox" in this folder.

    Include: contains the LAGraph.h file

    LICENSE: BSD 2-clause license

    Makefile: a simple Makefile that relies on CMake to build LAGraph.

    README.md: this file

    Source: source code for the LAGraph library

        * Algorithms: graph algorithms such as BFS, connected components,
            centrality, etc, will go here

        * Utilities: read/write a graph from a file, etc, will go here...
        

    Test: main programs that test LAGraph.  To run the tests, first compile
        GraphBLAS and LAGraph, and then do "make tests" in this directory.

    build: initially empty

To link against GraphBLAS, first install whatever GraphBLAS library you wish to
use.  LAGraph will use -lgraphblas and will include the GraphBLAS.h file
from its installed location.  Alternatively, the CMakeLists.txt script can use
a relative directory:

    ../GraphBLAS: any GraphBLAS implementation.

So that LAGraph and GraphBLAS reside in the same parent folder.  The include
file for GraphBLAS will be assumed to appear in ../GraphBLAS/Include, and the
compiled GraphBLAS library is assumed to appear in ../GraphBLAS/build.  If you
use a GraphBLAS library that uses a different structure, then edit the
CMakeLists.txt file to point to right location.

Authors: (... list them here)

