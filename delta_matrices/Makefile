CFLAGS = -D_GNU_SOURCE -fopenmp -g -O0

GRAPHBLAS=../deps/GraphBLAS/build/libgraphblas.a

# Sources
SOURCEDIR:=$(shell pwd -P)
CC_SOURCES = $(wildcard $(SOURCEDIR)/*.c)

all:
	$(CC) $(CFLAGS) -o delta_matrices $(CC_SOURCES) $(GRAPHBLAS) -lpthread -lm

clean:
	rm delta_matrices
