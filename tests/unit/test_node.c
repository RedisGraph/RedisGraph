#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../../src/graph/node.h"
#include "../../src/graph/edge.h"

void test_node_creation() {
	Node *node = NewNode(1l, "city");

	assert(node != NULL);
	assert(node->id == 1l);
	assert(node->prop_count == 0);
	assert(node->properties == NULL);
	assert(Vector_Size(node->outgoing_edges) == 0);
	assert(Vector_Size(node->incoming_edges) == 0);
	assert(strcmp(node->label, "city") == 0);
	FreeNode(node);
}

void test_node_props() {
	Node *node = NewNode(1l, "city");

	char *keys[2] = {"neighborhood", "block"};
	SIValue vals[2] = { SI_StringValC("rambam"), SI_IntVal(10)};

	GraphEntity_Add_Properties((GraphEntity*)node, 2, keys, vals);

	SIValue *val = GraphEntity_Get_Property((GraphEntity*)node, "neighborhood");
	assert(strcmp(val->stringval.str, "rambam") == 0);
	
	val = GraphEntity_Get_Property((GraphEntity*)node, "block");
	assert(val->intval == 10);

	val = GraphEntity_Get_Property((GraphEntity*)node, "fake");
	assert(val == PROPERTY_NOTFOUND);

	FreeNode(node);
}

void test_node_edges() {
	Node *src = NewNode(1l, "city");
	Node *dest = NewNode(2l, "flight");
	Edge *edge = NewEdge(3l, src, dest, "lands");

	Node_ConnectNode(src, dest, edge);
	
	assert(Node_IncomeDegree(src) == 0);
	assert(Node_IncomeDegree(dest) == 1);

	FreeNode(src);
	FreeNode(dest);
	FreeEdge(edge);
}

int main(int argc, char **argv) {
	test_node_creation();
	test_node_props();
	test_node_edges();
	printf("test_node - PASS!\n");
    return 0;
}