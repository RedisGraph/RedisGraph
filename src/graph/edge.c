#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"

Edge* NewEdge(long int id, Node *src, Node *dest, const char *relationship) {
	assert(src && dest);

	Edge* edge = (Edge*)calloc(1, sizeof(Edge));
	edge->id = id;
	edge->src = src;
	edge->dest = dest;
	edge->prop_count = 0;

	if(relationship != NULL) {
		edge->relationship = strdup(relationship);
	}

	return edge;
}

void Edge_Add_Properties(Edge *edge, int prop_count, char **keys, SIValue *values) {
	GraphEntity_Add_Properties((GraphEntity*)edge, prop_count, keys, values);
}

SIValue* Edge_Get_Property(const Edge *edge, const char* key) {
	return GraphEntity_Get_Property((GraphEntity*)edge, key);
}

void FreeEdge(Edge* edge) {
	FreeGraphEntity((GraphEntity*)edge);

	if(edge->relationship != NULL) {
		free(edge->relationship);
	}
	
	free(edge);
}