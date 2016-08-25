#include <stdio.h>
#include <string.h>
#include "filter.h"
#include "assert.h"
#include "node.h"

int main(int argc, char **argv) {
	
	Node* node = NewNode("");
	assert(strcmp(node->name, "") == 0);
	FreeNode(node);

	node = NewNode("origin");

	NodeAddFilter(node, EqualFilter("position", 0));

	assert(strcmp(node->name, "origin") == 0);
	assert(ValidateNode(node) == 1);
	assert(Vector_Size(node->outgoingEdges) == 0);

	FreeNode(node);

	printf("PASS!");
    return 0;
}