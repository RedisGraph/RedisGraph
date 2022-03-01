#-------------------------------------------------------------------------------
# GraphBLAS/GraphBLAS/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2022, All Rights Reserved.
# SPDX-License-Identifier: GPL-3.0-or-later

#-------------------------------------------------------------------------------

# simple Makefile for GraphBLAS, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

JOBS ?= 8

default: library

# just build the dynamic library; do not run the demo
library:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; $(MAKE) --jobs=$(JOBS) )

# compile with -g 
debug:
	( cd build ; cmake -DCMAKE_BUILD_TYPE=Debug $(CMAKE_OPTIONS) .. ; $(MAKE) --jobs=$(JOBS) )

# just do 'make' in build; do not rerun the cmake script
remake:
	( cd build ; $(MAKE) --jobs=$(JOBS) )

# just run cmake; do not compile
cmake:
	( cd build ; cmake $(CMAKE_OPTIONS) .. ; )

# installs GraphBLAS to the install location defined by cmake, usually
# /usr/local/lib and /usr/local/include
install:
	( cd build ; $(MAKE) install )

# remove any installed libraries and #include files
uninstall:
	- xargs rm < build/install_manifest.txt

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	rm -rf build/*
	( cd test/tcov ; $(MAKE) distclean )
	( cd @GrB/private ; $(MAKE) distclean )

