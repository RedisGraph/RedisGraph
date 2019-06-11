#-------------------------------------------------------------------------------
# LAGraph/Test/Makefile
#-------------------------------------------------------------------------------

# LAGraph, (... list all authors here) (c) 2019, All Rights Reserved.
# http://graphblas.org  See LAGraph/LICENSE for license.

#-------------------------------------------------------------------------------

# simple Makefile for LAGraph, relies on cmake to do the actual build.  Use
# the CMAKE_OPTIONS argument to this Makefile to pass options to cmake.

# Install GraphBLAS before trying to compile LAGraph.

JOBS ?= 1

# build all the tests
default:
	( cd LCC           ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd AllKTruss     ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd KTruss        ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd TriangleCount ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd PageRank      ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd BFS           ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd CC		   ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd MatrixMarket  ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )
	( cd BuildMatrix   ; $(MAKE) --jobs=$(JOBS) CC=$(CC) CXX=$(CC) )

clean: distclean

purge: distclean

# remove all files not in the distribution
distclean:
	( cd LCC           ; $(MAKE) distclean )
	( cd AllKTruss     ; $(MAKE) distclean )
	( cd KTruss        ; $(MAKE) distclean )
	( cd TriangleCount ; $(MAKE) distclean )
	( cd PageRank      ; $(MAKE) distclean )
	( cd BFS           ; $(MAKE) distclean )
	( cd CC            ; $(MAKE) distclean )
	( cd MatrixMarket  ; $(MAKE) distclean )
	( cd BuildMatrix   ; $(MAKE) distclean )

