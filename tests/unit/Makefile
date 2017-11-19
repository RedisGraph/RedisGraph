CFLAGS:=$(CFLAGS)
CC_OBJECTS:=$(CC_OBJECTS)
LIBRMUTIL=../../src/rmutil/librmutil.a
LIBTRIEMAP=../../src/util/triemap/libtriemap.a

LIBS=$(LIBTRIEMAP) $(LIBRMUTIL)
DEPS=$(CC_OBJECTS) $(LIBS)

SRCDIR := $(shell pwd)
LDFLAGS :=  -lc -lm -ldl -lpthread

CC=gcc
.SUFFIXES: .c .so .xo .o

agg_sum: test_agg_sum.o
	$(CC) $(CFLAGS)  -o test_agg_sum test_agg_sum.o $(DEPS) $(LDFLAGS)
test_agg_sum:
	@(sh -c ./test_agg_sum)
.PHONY: test_agg_sum

edge: test_edge.o
	$(CC) $(CFLAGS)  -o test_edge test_edge.o  $(DEPS) $(LDFLAGS)
test_edge:
	@(sh -c ./test_edge)
.PHONY: test_edge

filter_tree: test_filter_tree.o
	$(CC) $(CFLAGS)  -o test_filter_tree test_filter_tree.o  $(DEPS) $(LDFLAGS)
test_filter_tree:
	@(sh -c ./test_filter_tree)
.PHONY: test_filter_tree

graph: test_graph.o
	$(CC) $(CFLAGS)  -o test_graph test_graph.o  $(DEPS) $(LDFLAGS)
test_graph:
	@(sh -c ./test_graph)
.PHONY: test_graph

hexastore: test_hexastore.o
	$(CC) $(CFLAGS)  -o test_hexastore test_hexastore.o  $(DEPS) $(LDFLAGS)
test_hexastore:
	@(sh -c ./test_hexastore)
.PHONY: test_hexastore

node: test_node.o
	$(CC) $(CFLAGS)  -o test_node test_node.o  $(DEPS) $(LDFLAGS)
test_node:
	@(sh -c ./test_node)
.PHONY: test_node

resultset_record: test_resultset_record.o
	$(CC) $(CFLAGS)  -o test_resultset_record test_resultset_record.o  $(DEPS) $(LDFLAGS)
test_resultset_record:
	@(sh -c ./test_resultset_record)
.PHONY: test_resultset_record

triplet: test_triplet.o
	$(CC) $(CFLAGS)  -o test_triplet test_triplet.o  $(DEPS) $(LDFLAGS)
test_triplet:
	@(sh -c ./test_triplet)
.PHONY: test_triplet

value: test_value.o
	$(CC) $(CFLAGS)  -o test_value test_value.o  $(DEPS) $(LDFLAGS)
test_value:
	@(sh -c ./test_value)
.PHONY: test_value

arithmetic_expression: test_arithmetic_expression.o
	$(CC) $(CFLAGS)  -o test_arithmetic_expression test_arithmetic_expression.o  $(DEPS) $(LDFLAGS)
test_arithmetic_expression:
	@(sh -c ./test_arithmetic_expression)
.PHONY: test_arithmetic_expression

all: build test

build: agg_sum edge node graph value triplet resultset_record hexastore filter_tree arithmetic_expression

test: test_agg_sum test_edge test_node test_graph test_value test_triplet test_resultset_record test_hexastore test_filter_tree test_arithmetic_expression
