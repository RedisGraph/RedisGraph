#include <stdio.h>
#include <stdlib.h>
#include "edge.h"

Edge* NewEdge(Node* src, Node* dest, const char* relationship) {
	Edge* edge = (Edge*)malloc(sizeof(Edge));
	
	edge->src = src;
	edge->dest = dest;
	edge->relationship = (char*)malloc(strlen(relationship) + 1);
	strcpy(edge->relationship, relationship);

	return edge;
}

int ValidateEdge(const Edge* edge) {
	if(edge->src == NULL) {
		fprintf(stderr, "edge missing source node\n");
		return 0;
	}

	if(edge->dest == NULL) {
		fprintf(stderr, "edge missing destination node\n");
		return 0;
	}

	return 1;
}

void FreeEdge(Edge* edge) {
	free(edge->relationship);
	free(edge);
}