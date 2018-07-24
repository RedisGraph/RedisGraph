/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#include <stdio.h>
#include <string.h>
#include "../../src/graph/node.h"
#include "../../src/graph/edge.h"
#include "assert.h"

void test_edge_creation() {
	Node *src_node = NewNode(1l, "person");
	Node *dest_node = NewNode(2l, "city");
	Edge *edge = NewEdge(3l, src_node, dest_node, "lives");

	assert(edge->src == src_node);
	assert(edge->dest == dest_node);
	assert(strcmp(edge->relationship, "lives") == 0);
	assert(edge->prop_count == 0);

	Node_Free(src_node);
	Node_Free(dest_node);
	Edge_Free(edge);
}

void test_edge_props() {
	Node *src_node = NewNode(1l, "person");
	Node *dest_node = NewNode(2l, "city");
	Edge *edge = NewEdge(3l, src_node, dest_node, "lives");

	char *keys[2] = {"rent", "since"};
	SIValue vals[2] = { SI_BoolVal(1), SI_UintVal(1984)};

	GraphEntity_Add_Properties((GraphEntity*)edge, 2, keys, vals);

	SIValue *val = GraphEntity_Get_Property((GraphEntity*)edge, "rent");
	assert(val->boolval == 1);
	
	val = GraphEntity_Get_Property((GraphEntity*)edge, "since");
	assert(val->intval == 1984);

	val = GraphEntity_Get_Property((GraphEntity*)edge, "fake");
	assert(val == PROPERTY_NOTFOUND);

	Node_Free(src_node);
	Node_Free(dest_node);
	Edge_Free(edge);
}

int main(int argc, char **argv) {	
	test_edge_creation();
	test_edge_props();
	printf("test_edge - PASS!\n");
	return 0;
}
