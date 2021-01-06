#-------------------------------------------------------------------------------
# GraphBLAS/alternative/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

# To compile with 8 threads:
#
#   make -j8
#
# To install: do not just do 'sudo make install' since MKLROOT might be 
# defined for the user but not for root.  Instead do:
#
#   make -j8
#   sudo make install

default: library

VER1 = 4
VER2 = 0
VER3 = 1

# pick your compiler:
  CC = gcc
# CC = cc
# CC = clang
# CC = xlc
# CC = gcc-8
# note that -mp1 is essential for icc, for proper Inf and NaN behavior:
# CC = icc -mp1

# A C++ compiler works but is not recommended for use in production,
# particularly if SuiteSparse:GraphBLAS is used as -lgraphblas.
# SuiteSparse:GraphBLAS is written in C, not C++.  However, it compiles just
# fine with a C++ compiler, since it is written so that it uses the
# intersection of the two languages.  This is helpful for two cases: (1)
# SuiteSparse:GraphBLAS does a lot of typecasting, and C++ is very strict with
# this.  Using C++ allows for an extra-careful 'lint' checking.  (2) The end-
# user may wish to construct a pure C++ application, and embed a C++-compiled
# GraphBLAS inside.  CMake cannot be used to compile a *.c code with a C++
# compiler (it complains), and thus this option is only available in this
# alternative/Makefile.  The GCC C++ 5.4 compiler fails; version 7.5 is
# sufficient.
# CC = c++

# Using the Intel MKL (don't try this if CC=c++, it will likely break):
# from https://software.intel.com/content/www/us/en/develop/articles/intel-mkl-link-line-advisor.html
# MKL_CFLAGS = -DMKL_ILP64 -m64 -I${MKLROOT}/include -DGB_HAS_CBLAS
# MKL_LDFLAGS= -L${MKLROOT}/lib/intel64 -Wl,--no-as-needed -lmkl_intel_ilp64 -lmkl_gnu_thread -lmkl_core -lgomp -lpthread -lm -ldl

# without the Intel MKL (default):
  MKL_CFLAGS = 
  MKL_LDFLAGS=

SRC = ../Source/*.c ../Source/Generated/*.c
INC = ../Include/*.h ../Source/*.h ../Source/Template/* ../Source/Generated/*.h ../Source/Generator/*.h 
SRC2 = $(notdir $(wildcard $(SRC)))
OBJ = $(SRC2:.c=.o)
LDFLAGS = -fopenmp -lm $(MKL_LDFLAGS)
CFLAGS = -fopenmp -fexceptions -fPIC $(MKL_CFLAGS)
# pick the roptimization level:
  CFLAGS += -O3
# CFLAGS += -g
ifneq ($(CC),c++)
    # comment this out if using c++:
    CFLAGS += -std=c11 
endif
CPPFLAGS = -I../Include -I../Source -I../Source/Template -I../Source/Generated -I../Source/Generator
SO_OPTS = $(LDFLAGS)
CFLAGS += -Wno-pragmas

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
    # Mac
    CFLAGS += -fno-common
    SO_NAME = libgraphblas.dylib.$(VER1).$(VER2).$(VER3)
    SO_NAME0 = libgraphblas.dylib
    SO_NAME1 = libgraphblas.dylib.$(VER1)
    SO_OPTS += -dynamiclib -shared  -Wl,-install_name -Wl,$(SO_NAME1) -undefined dynamic_lookup
else
    # Linux
    SO_NAME = libgraphblas.so.$(VER1).$(VER2).$(VER3)
    SO_NAME0 = libgraphblas.so
    SO_NAME1 = libgraphblas.so.$(VER1)
    SO_OPTS += -shared -Wl,-soname -Wl,$(SO_NAME1)
endif

%.o: ../Source/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

%.o: ../Source/Generated/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

library: $(SO_NAME)
	ln -sf $(SO_NAME) $(SO_NAME0)
	ln -sf $(SO_NAME) $(SO_NAME1)

$(SO_NAME): $(OBJ)
	$(CC) $(SO_OPTS) $^ -o $@

.KEEP: $(OBJ)

static: libgraphblas.a

libgraphblas.a: $(OBJ)
	ar -rv $@ $^
	- ranlib $@

# Do "make" first, and then "sudo make install" (to set MKLROOT properly)
install: library
	cp $(SO_NAME) /usr/local/lib
	ln -sf /usr/local/lib/$(SO_NAME) /usr/local/lib/$(SO_NAME0)
	ln -sf /usr/local/lib/$(SO_NAME) /usr/local/lib/$(SO_NAME1)
	cp ../Include/GraphBLAS.h  /usr/local/include

DINC = ../Demo/Include/*.h $(INC)
DSRC = ../Demo/Source/*.c
DCPPFLAGS = $(CPPFLAGS) -I../Demo/Include
DLIBS = $(SO_NAME) -lm
DSRC2 = $(notdir $(wildcard $(DSRC)))
DOBJ = $(DSRC2:.c=.o)

.KEEP: $(DOBJ)

%.o: ../Demo/Source/%.c $(DINC)
	$(CC) -c $(CFLAGS) $(DCPPFLAGS) $< -o $(notdir $@)

%_demo: ../Demo/Program/%_demo.c $(SO_NAME) $(DINC) $(DOBJ)
	$(CC) $(CFLAGS) $(DCPPFLAGS) $< $(DOBJ) $(DLIBS) -o $@

DEMO_PRG = $(notdir $(wildcard ../Demo/Program/*_demo.c))
DEMO = $(DEMO_PRG:.c=)

demo: $(DEMO)

run: $(DEMO)
	./altdemo

clean:
	$(RM) -f *.o *.out *_out.m *_out2.m

distclean: clean
	$(RM) -rf *.dSYM $(DEMO) libgraphblas.*

purge: distclean

