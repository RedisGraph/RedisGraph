#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "../src/graph/node.h"

int main(int argc, char **argv) {
	
	// Empty node
	Node* node = NewNode("", "");
	assert(strcmp(node->id, "") == 0);
	FreeNode(node);

	// Named node with a single filter.
	node = NewNode("","origin");	
	assert(strcmp(node->id, "origin") == 0);
	FreeNode(node);

	printf("PASS!");
    return 0;
}