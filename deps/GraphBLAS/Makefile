#-------------------------------------------------------------------------------
# GraphBLAS/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2020, All Rights Reserved.
# http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# simple Makefile for GraphBLAS, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

# Find the OS.
uname_S := $(shell sh -c 'uname -s 2>/dev/null || echo not')
# Set number of threads.
ifeq ($(uname_S),Linux)
	JOBS ?= $(shell grep -c ^processor /proc/cpuinfo)
else
	JOBS ?= $(shell sysctl -n hw.logicalcpu_max)
endif

default: library

# just build the dynamic library; do not run the demo
library:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) --jobs=$(JOBS) )

# just run the demos: assumes the library is already compiled
run:
	( cd Demo ; ./demo )

# just do 'make' in build; do not rerun the cmake script
remake:
	( cd build ; $(MAKE) --jobs=$(JOBS) )

# just run cmake; do not compile
cmake:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; )

# build both the static and dynamic libraries do not run the demo
static:
	( cd build ; cmake $(CMAKE_OPTIONS) -DBUILD_GRB_STATIC_LIBRARY=1 .. ; $(MAKE) --jobs=$(JOBS) )

static_only:
	( cd build ; cmake $(CMAKE_OPTIONS) -DBUILD_GRB_STATIC_LIBRARY=1 .. ; $(MAKE) --jobs=$(JOBS) graphblas_static )

# installs GraphBLAS to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build ; $(MAKE) install )

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
	rm -rf build/* Demo/*.out Demo/complex_demo_out.m Tcov/log.txt
	rm -rf Config/*.tmp Source/control.m4
	rm -rf Doc/html/* Doc/*.tmp
	( cd GraphBLAS/test/tcov ; $(MAKE) distclean )
	( cd GraphBLAS/@GrB/private ; $(MAKE) distclean )
	( cd Test ; $(MAKE) distclean )
	( cd Tcov ; $(MAKE) distclean )
	( cd Doc  ; $(MAKE) distclean )
	( cd alternative  ; $(MAKE) distclean )

