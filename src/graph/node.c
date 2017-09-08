#include <stdlib.h>

#include "node.h"
#include "edge.h"
#include "assert.h"
#include "graph_entity.h"

// SIValue* NODE_PROPERTY_NOTFOUND = &SI_StringValC("NOT FOUND");

Node* NewNode(long int id, const char *label) {
	Node* node = (Node*)calloc(1, sizeof(Node));
	
	node->id = id;
	node->prop_count = 0;
	node->outgoingEdges = NewVector(Edge*, 0);
	node->incomingEdges = NewVector(Edge*, 0);
	
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
	Vector_Push(src->outgoingEdges, e);
	Vector_Push(dest->incomingEdges, e);
}

int Node_IncomeDegree(const Node *n) {
	return Vector_Size(n->incomingEdges);
}

void Node_Add_Properties(Node *node, int prop_ount, char **keys, SIValue *values) {
	GraphEntity_Add_Properties((GraphEntity*)node, prop_ount, keys, values);
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

	/* TODO: free edgs.
	 * for(int i = 0; i < Vector_Size(node->outgoingEdges); i++) {
	 * 	Edge* e;
	 * 	Vector_Get(node->outgoingEdges, i, &e);
	 * 	FreeEdge(e);
	 * } */
	Vector_Free(node->outgoingEdges);

	/* There's no need to discard incoming edges.
	 * these will be freed on another outgoingEdges free. */ 
	Vector_Free(node->incomingEdges);
	free(node);
	node = NULL;
}