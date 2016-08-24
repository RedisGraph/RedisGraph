#include <stdio.h>
#include <string.h>
#include "node.h"
#include "edge.h"
#include "graph.h"
#include "assert.h"

int main(int argc, char **argv) {

	Graph* graph = NewGraph();	
	Node *nodeUtah = GraphAddNode(graph, "Utah");
	Node *nodeWyoming = GraphAddNode(graph, "Wyoming");
	Edge* edge = GraphConnectNodes(graph, "Wyoming", "Utah", "neighbors");

	NodeAddFilter(nodeWyoming, GreaterThanFilter("population", 584000));

	assert(strcmp(nodeWyoming->name, "Wyoming") == 0);
	assert(Vector_Size(nodeWyoming->filters) == 1);
	assert(Vector_Size(nodeWyoming->incomingEdges) == 0);
	assert(Vector_Size(nodeWyoming->outgoingEdges) == 1);
	assert(Vector_Size(nodeUtah->incomingEdges) == 1);
	assert(Vector_Size(nodeUtah->outgoingEdges) == 0);
	assert(ValidateEdge(edge) == 1);
	assert(strcmp(edge->relationship, "neighbors") == 0);

	assert(Vector_Size(graph->nodes) == 2);
	assert(Vector_Size(graph->edges) == 1);

	assert(ValidateGraph(graph) == 1);

	FreeGraph(graph);

	printf("PASS!");
    return 0;
}