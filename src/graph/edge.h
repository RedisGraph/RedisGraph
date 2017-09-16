#ifndef EDGE_H_
#define EDGE_H_

#include "graph_entity.h"
#include "node.h"
#include "../value.h"

struct Edge {
	struct {
		long int id;
		int prop_count;
		EntityProperty *properties;
	};
	char* relationship;
	Node* src;
	Node* dest;
};

typedef struct Edge Edge;

/* Creates a new edge, connecting src to dest node. */
Edge* NewEdge(long int id, Node *src, Node *dest, const char *relationship);

/* Adds a properties to node
 * propCount - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void Edge_Add_Properties(Edge *edge, int propCount, char **keys, SIValue *values);

/* Retrieves edge's property
 * NOTE: If the key does not exist, we return the special
 * constant value Edge_PROPERTY_NOTFOUND. */
SIValue* Edge_Get_Property(const Edge *edge, const char *key);

// Frees alocated space by given edge
void FreeEdge(Edge *edge);

#endif