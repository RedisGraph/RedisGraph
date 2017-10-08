#include <stdlib.h>

#include "node.h"
#include "edge.h"
#include "assert.h"
#include "graph_entity.h"

Node* NewNode(long int id, const char *label) {
	Node* node = (Node*)calloc(1, sizeof(Node));
	
	node->id = id;
	node->prop_count = 0;
	node->outgoing_edges = NewVector(Edge*, 0);
	node->incoming_edges = NewVector(Edge*, 0);
	
	if(label != NULL) {
		node->label = strdup(label);
	}

	return node;
}

int Node_Compare(const Node *a, const Node *b) {
	return a->id == b->id;
}

void Node_ConnectNode(Node* src, Node* dest, struct Edge* e) {
	// assert(src && dest && e->src == src && e->dest == dest);
	Vector_Push(src->outgoing_edges, e);
	Vector_Push(dest->incoming_edges, e);
}

int Node_IncomeDegree(const Node *n) {
	return Vector_Size(n->incoming_edges);
}

void Node_Add_Properties(Node *node, int prop_count, char **keys, SIValue *values) {
	GraphEntity_Add_Properties((GraphEntity*)node, prop_count, keys, values);
}

SIValue* Node_Get_Property(const Node *node, const char* key) {
	return GraphEntity_Get_Property((GraphEntity*)node, key);
}

void FreeNode(Node* node) {	
	if(!node) return;

	FreeGraphEntity((GraphEntity*)node);

	if(node->label != NULL) {
		free(node->label);
	}

	Vector_Free(node->outgoing_edges);
	Vector_Free(node->incoming_edges);
	
	free(node);
	node = NULL;
}