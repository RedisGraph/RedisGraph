#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/graph/node.h"
#include "../src/graph/edge.h"

void testNodeConnect() {
	Node *a = NewNode("", "");
	Node *b = NewNode("", "");

	Edge *e = NewEdge("1", a, b, "relationship");
	ConnectNode(a, b, e);

	assert(Vector_Size(a->outgoingEdges) == 1);
	assert(Vector_Size(b->outgoingEdges) == 0);

	assert(Vector_Size(a->incomingEdges) == 0);
	assert(Vector_Size(b->incomingEdges) == 1);
	
	Vector_Get(a->outgoingEdges, 0, &e);

	assert(e->src == a);
	assert(e->dest == b);
	assert(strcmp(e->relationship, "relationship") == 0);

	FreeNode(a);
	FreeNode(b);
}

void TestNodeCreation() {
	// Empty node
	Node* node = NewNode("", "");
	assert(strcmp(node->alias, "") == 0);
	assert(strcmp(node->id, "") == 0);
	assert(Vector_Size(node->incomingEdges) == 0);
	assert(Vector_Size(node->outgoingEdges) == 0);
	FreeNode(node);

	// Named node with alias and id
	node = NewNode("alias", "id");
	assert(strcmp(node->alias, "alias") == 0);
	assert(strcmp(node->id, "id") == 0);
	assert(Vector_Size(node->incomingEdges) == 0);
	assert(Vector_Size(node->outgoingEdges) == 0);
	FreeNode(node);
}

int main(int argc, char **argv) {
	testNodeConnect();
	TestNodeCreation();
	printf("PASS!");
    return 0;
}