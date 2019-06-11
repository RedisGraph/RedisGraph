#-------------------------------------------------------------------------------
# LAGraph/Makefile
#-------------------------------------------------------------------------------

# LAGraph, (... list all authors here) (c) 2019, All Rights Reserved.
# http://graphblas.org  See LAGraph/LICENSE for license.

#-------------------------------------------------------------------------------

# simple Makefile for LAGraph, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

# Install GraphBLAS before trying to compile LAGraph.

JOBS ?= 1

# build the LAGraph library (static and dynamic)
library:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) --jobs=$(JOBS) )

# run all tests (do "make ; make tests")
tests:
	( cd Test ; $(MAKE) --jobs=$(JOBS) )

# just run cmake; do not compile
cmake:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; )

# the same as "make library"
static: library

# installs LAGraph to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build ; $(MAKE) install )

# TODO create the Doc/LAGraph.pdf
docs:
	( cd Doc ; $(MAKE) )

# create the doxygen documentation in Doc/html
dox:
	doxygen

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	rm -rf build/*
	rm -rf Doc/html/*
	( cd Doc  ; $(MAKE) distclean )
	( cd Test ; $(MAKE) distclean )

