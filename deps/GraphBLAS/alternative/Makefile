#-------------------------------------------------------------------------------
# GraphBLAS/alternative/Makefile
#-------------------------------------------------------------------------------

#  SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2019, All Rights Reserved.
#  http://suitesparse.com   See GraphBLAS/Doc/License.txt for license.

#-------------------------------------------------------------------------------

# pick your compiler:
# CC = gcc
# CC = cc
# CC = clang
# CC = xlc
# CC = gcc-8
  CC = icc -mp1

# note that -mp1 is essential for icc, for proper Inf and NaN behavior.

SRC = ../Source/*.c ../Source/Generated/*.c
INC = ../Include/*.h ../Source/*.h ../Source/Template/* ../Source/Generated/*.h ../Source/Generator/*.h 
SRC2 = $(notdir $(wildcard $(SRC)))
OBJ = $(SRC2:.c=.o)
LDFLAGS = -fopenmp
CFLAGS = -O3 -std=c11 -fopenmp -fexceptions -fPIC
CPPFLAGS = -I../Include -I../Source -I../Source/Template -I../Source/Generated -I../Source/Generator
SO_OPTS = $(LDFLAGS)

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
    # Mac
    CFLAGS += -fno-common
    SO_NAME = libgraphblas.dylib
    SO_OPTS += -dynamiclib -shared -undefined dynamic_lookup
else
    # Linux
    SO_NAME = libgraphblas.so
    SO_OPTS += -shared -Wl,-soname -Wl,$(SO_NAME) -Wl,--no-undefined
endif

%.o: ../Source/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

%.o: ../Source/Generated/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

default: $(SO_NAME)

$(SO_NAME): $(OBJ)
	$(CC) $(SO_OPTS) $^ -o $@

.KEEP: $(OBJ)

static: libgraphblas.a

libgraphblas.a: $(OBJ)
	ar -rv $@ $^
	- ranlib $@

install: $(SO_NAME)
	$(CP) $(SO_NAME) /usr/local/lib
	$(CP) ../Include/GraphBLAS.h  /usr/local/include

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

run: $(DEMO)
	./altdemo

clean:
	$(RM) -f *.o *.out *_out.m

distclean: clean
	$(RM) -rf *.dSYM $(DEMO) libgraphblas.*

purge: distclean

