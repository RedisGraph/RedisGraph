#-------------------------------------------------------------------------------
# GraphBLAS/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017, All Rights Reserved.
# http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# simple Makefile for GraphBLAS, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

# build the GraphBLAS library (static and dynamic) and run a quick test
default:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) ; cd ../Demo ; ./demo )

# just build the static and dynamic libraries; do not run the demo
library:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) )

# the same as "make library"
static: library

# installs GraphBLAS to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) ; $(MAKE) install )

# create the Doc/GraphBLAS_UserGuide.pdf
docs:
	( cd Doc ; $(MAKE) )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	rm -rf build/* Demo/*_demo.out Demo/complex_demo_out.m Tcov/log.txt
	( cd Test ; $(MAKE) distclean )
	( cd Tcov ; $(MAKE) distclean )
	( cd Doc  ; $(MAKE) distclean )

