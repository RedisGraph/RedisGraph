#-------------------------------------------------------------------------------
# GraphBLAS/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

# simple Makefile for GraphBLAS, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.
# For example, to compile with 40 threads, use:
#
#       make JOBS=40
#
# To compile without using Google's cpu_features package, using 40 threads:
#
#       make CMAKE_OPTIONS='-DGBNCPUFEAT=1' JOBS=40
#
# To use multiple options, separate them by a space.  For example, to build
# just the library but not cpu_features, and to enable AVX2 but not AVX512F:
#
#       make CMAKE_OPTIONS='-DGBNCPUFEAT=1 -DGBAVX2=1' JOBS=40

JOBS ?= 8

default: library

# just build the dynamic library, but not the demos
library:
	( cd build && cmake $(CMAKE_OPTIONS) .. && $(MAKE) --jobs=$(JOBS) )

# enable CUDA (NOTE: not ready for production use)
cuda:
	( cd build && cmake $(CMAKE_OPTIONS) -DENABLE_CUDA=1 .. && $(MAKE) --jobs=$(JOBS) )

# install only in SuiteSparse/lib and SuiteSparse/include
local:
	( cd build && cmake $(CMAKE_OPTIONS) -DSUITESPARSE_LOCAL=1 .. && $(MAKE) --jobs=${JOBS} )

# compile with -g 
debug:
	( cd build && cmake -DCMAKE_BUILD_TYPE=Debug $(CMAKE_OPTIONS) .. && $(MAKE) --jobs=$(JOBS) )

# build the dynamic library and the demos
all:
	( cd build && cmake $(CMAKE_OPTIONS) -DDEMO=1 .. && $(MAKE) --jobs=$(JOBS) )

# run the demos
demo: all
	( cd Demo && ./demo )

# just do 'make' in build; do not rerun the cmake script
remake:
	( cd build && $(MAKE) --jobs=$(JOBS) )

# just run cmake; do not compile
setup:
	( cd build && cmake $(CMAKE_OPTIONS) .. )

# build both the static and dynamic libraries; do not run the demo
static:
	( cd build && cmake $(CMAKE_OPTIONS) -DBUILD_GRB_STATIC_LIBRARY=1 .. && $(MAKE) --jobs=$(JOBS) )

static_only:
	( cd build && cmake $(CMAKE_OPTIONS) -DBUILD_GRB_STATIC_LIBRARY=1 .. && $(MAKE) --jobs=$(JOBS) graphblas_static )

# installs GraphBLAS to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build && $(MAKE) install )

# create the Doc/GraphBLAS_UserGuide.pdf
docs:
	( cd Doc && $(MAKE) )

# compile the CUDA kernels
gpu:
	( cd CUDA && $(MAKE) )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	- rm -rf build/* Demo/*.out Demo/complex_demo_out*.m Tcov/log.txt
	- rm -rf Config/*.tmp Source/control.m4
	- rm -rf Doc/html/* Doc/*.tmp
	( cd GraphBLAS && $(MAKE) distclean )
	( cd Test && $(MAKE) distclean )
	( cd Tcov && $(MAKE) distclean )
	( cd Doc  && $(MAKE) distclean )
	( cd alternative && $(MAKE) distclean )

