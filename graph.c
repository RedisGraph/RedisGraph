#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "graph.h"

Node* NewNode(const char* name) {
	Node* node = (Node*)malloc(sizeof(Node));
	node->filters = NewVector(Filter*, 0); // Vector initial capacity 0.
	node->incomingEdges = NewVector(Edge*, 0); // Vector initial capacity 0.
	node->outgoingEdges = NewVector(Edge*, 1); // Vector initial capacity 1.
	node->name = (char*)malloc(sizeof(char) * (strlen(name) + 1));

	strcpy(node->name, name);

	return node;
}

void NodeAddFilter(const Node* node, const Filter* filter) {
	Vector_Push(node->filters, filter);
}

int ValidateNode(const Node* node) {
	// Check if node's filters are valid.
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        Filter *f;
        Vector_Get(node->filters, i, &f);
        if(ValidateFilter(f) == 0) {
        	return 0;
        }
    }

	return 1;
}

void FreeNode(Node* node) {
	free(node->name);

	// Free filters
	for (int i = 0; i < Vector_Size(node->filters); i++) {
        Filter* f;
        Vector_Get(node->filters, i, &f);
        FreeFilter(f);        
    }
    
	Vector_Free(node->filters);
	Vector_Free(node->incomingEdges);
	Vector_Free(node->outgoingEdges);
	free(node);
}

Edge* NewEdge(const Node* src, const Node* dest, const char* relationship) {
	Edge* edge = (Edge*)malloc(sizeof(Edge));
	
	edge->src = src;
	edge->dest = dest;	
	edge->relationship = (char*)malloc(strlen(relationship) + 1);
	strcpy(edge->relationship, relationship);

	return edge;
}

int ValidateEdge(const Edge* edge) {
	if(edge->src == 0) {
		fprintf(stderr, "edge missing source node\n");
		return 0;
	}

	if(edge->dest == 0) {
		fprintf(stderr, "edge missing destination node\n");
		return 0;
	}

	return 1;
}

void FreeEdge(Edge* edge) {
	free(edge->relationship);
	free(edge);
}

Graph* NewGraph() {
	Graph* graph = (Graph*)malloc(sizeof(Graph));
	graph->nodes = NewVector(Node*, 2);
	graph->edges = NewVector(Edge*, 1);
	return graph;
}

Node* GraphAddNode(const Graph* graph, const char* nodeName) {
	Node* node = NewNode(nodeName);
	Vector_Push(graph->nodes, node);
	return node;
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
	Vector_Push(nodeB->incomingEdges, edge);
	Vector_Push(graph->edges, edge);

	return edge;
}

int ValidateGraph(const Graph* graph) {
	for (int i = 0; i < Vector_Size(graph->nodes); i++) {
        Node* node;
        Vector_Get(graph->nodes, i, &node);
        // Check if node has no edges coming in or out of it.
		if(Vector_Size(node->incomingEdges) == 0 &&
			Vector_Size(node->outgoingEdges) == 0) {
			fprintf(stderr, "Disconnected, floating nodes are not allowed\n");
			return 0;
		}
    }
	
	return 1;
}

void FreeGraph(Graph* graph) {
	for (int i = 0; i < Vector_Size(graph->nodes); i++) {
        Node *node;
        Vector_Get(graph->nodes, i, &node);
        FreeNode(node);
    }

    for (int i = 0; i < Vector_Size(graph->edges); i++) {
        Edge *edge;
        Vector_Get(graph->edges, i, &edge);
        FreeEdge(edge);
    }

	Vector_Free(graph->nodes);
	Vector_Free(graph->edges);
	free(graph);
}