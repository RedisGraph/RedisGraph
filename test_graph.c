#include <stdio.h>
#include <string.h>
#include "graph.h"
#include "assert.h"

int main(int argc, char **argv) {
	
	Node *nodeSrc = NewNode("Wyoming");
	NodeAddFilter(nodeSrc, GreaterThanFilter("population", 584000));

	Node *nodeDest = NewNode("Utah");
	NodeAddFilter(nodeDest, GreaterThanFilter("population", 2000000));

	Edge* edge = ConnectNodes(nodeSrc, nodeDest, "neighbors");
	
	assert(strcmp(nodeSrc->name, "Wyoming") == 0);
	assert(ValidateNode(nodeSrc) == 1);
	assert(Vector_Size(nodeSrc->filters) == 1);
	assert(Vector_Size(nodeSrc->incomingEdges) == 0);
	assert(Vector_Size(nodeSrc->outgoingEdges) == 1);
	assert(Vector_Size(nodeDest->incomingEdges) == 1);
	assert(Vector_Size(nodeDest->outgoingEdges) == 0);
	assert(ValidateEdge(edge) == 1);
	assert(strcmp(edge->relationship, "neighbors") == 0);

	FreeNode(nodeSrc);
	FreeNode(nodeDest);
	FreeEdge(edge);

	printf("PASS!");
    return 0;
}