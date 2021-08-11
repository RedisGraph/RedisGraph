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
# To install:
#
#   make -j8
#   sudo make install

default: library

VER1 = 5
VER2 = 1
VER3 = 4

# pick your compiler:
  CC = gcc
# CC = cc
# CC = clang
# CC = xlc
# CC = gcc-8
# note that -mp1 is essential for icc, for proper Inf and NaN behavior:
# CC = icc -mp1
# CC = c++

SRC = ../Source/*.c ../Source/Generated1/*.c ../Source/Generated2/*.c
INC = ../Include/*.h ../Source/*.h ../Source/Template/* ../Source/Generated1/*.h ../Source/Generated2/*.h
SRC2 = $(notdir $(wildcard $(SRC)))
OBJ = $(SRC2:.c=.o)
LDFLAGS = -fopenmp -lm
CFLAGS = -fopenmp -fexceptions -fPIC
# pick the optimization level:
  CFLAGS += -O3
# CFLAGS += -g
ifneq ($(CC),c++)
    # comment this out if using c++:
    CFLAGS += -std=c11 
endif
CPPFLAGS = -I../Include -I../Source -I../Source/Template -I../Source/Generated1 -I../Source/Generated2
SO_OPTS = $(LDFLAGS)
CFLAGS += -Wno-pragmas

# To compile the libgraphblas_renamed library, change all occurences of
# libgraphblas to libgraphblas_renamed, below, and uncomment these 2 lines:
# CFLAGS += -DGBRENAME=1
# CPPFLAGS += -I../GraphBLAS/rename

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

%.o: ../Source/Generated1/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

%.o: ../Source/Generated2/%.c $(INC)
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

# Do "make" first, and then "sudo make install"
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

