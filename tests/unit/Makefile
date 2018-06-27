CFLAGS:=$(CFLAGS)
CC_OBJECTS:=$(CC_OBJECTS)
LIBRMUTIL=../../src/rmutil/librmutil.a
LIBTRIEMAP=../../src/util/triemap/libtriemap.a
LIBGRAPHBLAS=../../deps/GraphBLAS/build/libgraphblas.a

LIBS=$(LIBTRIEMAP) $(LIBRMUTIL) $(LIBGRAPHBLAS)
DEPS=$(CC_OBJECTS) $(LIBS)

SRCDIR := $(shell pwd)
LDFLAGS :=  -lc -lm -ldl -lpthread

CC=gcc
.SUFFIXES: .c .so .xo .o

graph: test_graph.o
	$(CC) $(CFLAGS) -o test_graph test_graph.o  $(DEPS) $(LDFLAGS)

algebraic_expression: test_algebraic_expression.o
	$(CC) $(CFLAGS) -o test_algebraic_expression test_algebraic_expression.o  $(DEPS) $(LDFLAGS)

tuples_iter: test_tuples_iter.o
	$(CC) $(CFLAGS) -o test_tuples_iter test_tuples_iter.o  $(DEPS) $(LDFLAGS)

filter_tree: test_filter_tree.o
	$(CC) $(CFLAGS) -o test_filter_tree test_filter_tree.o  $(DEPS) $(LDFLAGS)

skiplist_generic: test_skiplist_generic.o
	$(CC) $(CFLAGS)  -o test_skiplist_generic test_skiplist_generic.o  $(DEPS) $(LDFLAGS)

skiplist_graph: test_skiplist_graph.o
	$(CC) $(CFLAGS)  -o test_skiplist_graph test_skiplist_graph.o  $(DEPS) $(LDFLAGS)

value: test_value.o
	$(CC) $(CFLAGS)  -o test_value test_value.o  $(DEPS) $(LDFLAGS)

arithmetic_expression: test_arithmetic_expression.o
	$(CC) $(CFLAGS)  -o test_arithmetic_expression test_arithmetic_expression.o  $(DEPS) $(LDFLAGS)

resultset_record: test_resultset_record.o
	$(CC) $(CFLAGS)  -o test_resultset_record test_resultset_record.o  $(DEPS) $(LDFLAGS)

build: graph algebraic_expression tuples_iter filter_tree value arithmetic_expression resultset_record skiplist_generic skiplist_graph

test_graph:
	@(sh -c ./test_graph)
.PHONY: test_graph

test_algebraic_expression:
	@(sh -c ./test_algebraic_expression)
.PHONY: test_algebraic_expression

test_tuples_iter:
	@(sh -c ./test_tuples_iter)
.PHONY: test_tuples_iter

test_filter_tree:
	@(sh -c ./test_filter_tree)
.PHONY: test_filter_tree

test_skiplist_generic:
	@(sh -c ./test_skiplist_generic)
.PHONY: test_skiplist_generic

test_skiplist_graph:
	@(sh -c ./test_skiplist_graph)
.PHONY: test_skiplist_graph

test_value:
	@(sh -c ./test_value)
.PHONY: test_value

test_arithmetic_expression:
	@(sh -c ./test_arithmetic_expression)
.PHONY: test_arithmetic_expression

test_resultset_record:
	@(sh -c ./test_resultset_record)
.PHONY: test_resultset_record

test: test_graph test_algebraic_expression test_tuples_iter test_filter_tree test_value test_arithmetic_expression test_resultset_record test_skiplist_generic test_skiplist_graph

all: build test