#-------------------------------------------------------------------------------
# GraphBLAS/Tcov/Makefile
#-------------------------------------------------------------------------------

# SuiteSparse:GraphBLAS, Timothy A. Davis, (c) 2017-2021, All Rights Reserved.
# SPDX-License-Identifier: Apache-2.0

#-------------------------------------------------------------------------------

UNAME := $(shell uname)

# pick your compiler:
# CC = gcc
# CC = cc
# CC = clang
# CC = xlc
# CC = gcc-8
# CC = icc
ifeq ($(UNAME),Darwin)
    CC = gcc-8
else
    CC = gcc
endif

SRC = tmp_source/*
INC = tmp_include/*
SRC2 = $(notdir $(wildcard $(SRC)))
OBJ = $(SRC2:.c=.o)
LDFLAGS = -fopenmp
CFLAGS = -g -std=c11 -fopenmp -fexceptions -fPIC -Wno-pragmas
CPPFLAGS = -Itmp_include -DGBCOVER $(RENAME)
SO_OPTS = $(LDFLAGS)

ifeq ($(UNAME),Darwin)
    # Mac
    CFLAGS += -fno-common
    SO_NAME = libgraphblas_tcov.dylib
    SO_OPTS += -dynamiclib -shared -Wl,-install_name -Wl,$(SO_NAME) -undefined dynamic_lookup
else
    # Linux
    SO_NAME = libgraphblas_tcov.so
    SO_OPTS += -shared -Wl,-soname -Wl,$(SO_NAME) -Wl,--no-undefined
endif

default: $(SO_NAME)

%.o: tmp_source/%.c $(INC)
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $(notdir $@)

$(SO_NAME): $(OBJ)
	$(CC) $(SO_OPTS) $^ -o $@ -lm

.KEEP: $(OBJ)

clean:
	$(RM) *.o

distclean: clean
	$(RM) *.mex* tmp*/* errlog*.txt grbstat.mat fprint.txt
	$(RM) libgraphblas_tcov.*

purge: distclean
	$(RM) log.txt

config:
	@echo "CFLAGS:   " $(CFLAGS)
	@echo "CPPFLAGS: " $(CPPFLAGS)
	@echo "LDFLAGS:  " $(LDFLAGS)
