#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "graph.h"

Graph* NewGraph() {
	Graph* graph = (Graph*)malloc(sizeof(Graph));
	graph->nodes = NewVector(Node*, 2);
	return graph;
}

Node* GraphAddNode(const Graph* graph, const char* nodeName) {
	Node* node = NewNode(nodeName);
	Vector_Push(graph->nodes, node);
	return node;
}

Node* GraphRemoveNode(Graph* graph, const char* nodeName) {
	
	// Create a new vector.
	Vector * newVector = NewVector(Node*, Vector_Size(graph->nodes)-1);
	Node* removedNode = 0;

	while(Vector_Size(graph->nodes) > 0) {
        Node* currentNode;
        Vector_Pop(graph->nodes, &currentNode);
        
        if(strcmp(currentNode->name, nodeName) == 0) {
        	removedNode = currentNode;
        	continue;
        }

        Vector_Push(newVector, currentNode);
    }

    Vector_Free(graph->nodes);

    // Remove all node edges
    if(removedNode != 0) {
		while(Vector_Size(removedNode->outgoingEdges) > 0) {
			Edge *edge;
			Vector_Pop(removedNode->outgoingEdges, &edge);
			FreeEdge(edge);
		}
	}

    graph->nodes = newVector;

    return removedNode;
}

Node* GraphGetNode(const Graph* graph, const char* nodeName) {
	for (int i = 0; i < Vector_Size(graph->nodes); i++) {
        Node* node;
        Vector_Get(graph->nodes, i, &node);
        if(strcmp(node->name, nodeName) == 0) {
        	return node;
        }
    }

    return 0;
}

// Assuming nodes A and B are already in the graph.
Edge* GraphConnectNodes(const Graph * graph, const char* a, const char* b, const char* relationship) {
	Node* nodeA = GraphGetNode(graph, a);
	Node* nodeB = GraphGetNode(graph, b);

	if(nodeA == 0 || nodeB == 0) {
		return 0;
	}

	Edge* edge = NewEdge(nodeA, nodeB, relationship);

	Vector_Push(nodeA->outgoingEdges, edge);

	return edge;
}

int ValidateGraph(const Graph* graph) {	
    // Make sure each graph doesn't contain cycles.	
	return 1;
}

void FreeGraph(Graph* graph) {
	for (int i = 0; i < Vector_Size(graph->nodes); i++) {
		Node *node;
		Vector_Get(graph->nodes, i, &node);

		// Free node's outgoing edges.
		for(int j = 0; j < Vector_Size(node->outgoingEdges); j++) {
			Edge *edge;
			Vector_Get(node->outgoingEdges, j, &edge);
			FreeEdge(edge);
		}

		// Frees internal vectors.
        FreeNode(node);
    }

    Vector_Free(graph->nodes);
	free(graph);
}