#include <stdio.h>
#include <stdlib.h>
#include "edge.h"

Edge* NewEdge(const char *id, Node *src, Node *dest, const char *relationship) {
	Edge* edge = (Edge*)calloc(1, sizeof(Edge));
	
	edge->src = src;
	edge->dest = dest;
	edge->relationship = strdup(relationship);

	if(id != NULL) {
		edge->id = strdup(id);
	}

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
	if(edge->id != NULL) {
		free(edge->id);
	}
	if(edge->relationship != NULL) {
		free(edge->relationship);
	}
	free(edge);
}