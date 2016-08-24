#include <stdio.h>
#include <string.h>
#include "assert.h"
#include "node.h"
#include "edge.h"

int main(int argc, char **argv) {

	Node* src = NewNode("source");
	Node* dest = NewNode("destination");
	Edge* edge = NewEdge(src, dest, "route");


	assert(strcmp(edge->relationship, "route") == 0);
	assert(ValidateEdge(edge) == 1);
	assert(edge->src == src);
	assert(edge->dest == dest);

	assert(strcmp(edge->src->name, "source") == 0);
	assert(strcmp(edge->dest->name, "destination") == 0);

	FreeEdge(edge);

	printf("PASS!");
    return 0;
}