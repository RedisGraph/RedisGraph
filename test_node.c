#include <stdio.h>
#include <string.h>
#include "filter.h"
#include "assert.h"
#include "node.h"

int main(int argc, char **argv) {
	
	// Empty node
	Node* node = NewNode("");
	assert(strcmp(node->name, "") == 0);
	FreeNode(node);

	// Named node with a single filter.
	node = NewNode("origin");
	NodeAddFilter(node, "position", EqualIntegerFilter(0));

	assert(strcmp(node->name, "origin") == 0);0
	assert(Vector_Size(node->filters) == 1);
	assert(Vector_Size(node->outgoingEdges) == 0);

	NodeAddFilter(node, "radius", GreaterThanIntergerFilter(2, 1));
	assert(Vector_Size(node->filters) == 2);
	assert(ValidateNode(node) == 1);

	FreeNode(node);

	printf("PASS!");
    return 0;
}