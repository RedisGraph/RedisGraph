/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Apache License, Version 2.0,
* modified with the Commons Clause restriction.
*/

#ifndef EDGE_H_
#define EDGE_H_

#include "node.h"
#include "../value.h"
#include "graph_entity.h"
#include "../util/uthash.h"
#include "../../deps/GraphBLAS/Include/GraphBLAS.h"

struct Edge {
	struct {
		EdgeID id;
		int prop_count;
		EntityProperty *properties;
	};
	NodeID src_id;				// ID of source node.
	NodeID dest_id;				// ID of destination node.
	int relationship_id;		// ID of relationship matrix.
	char* relationship;
	Node* src;
	Node* dest;
	GrB_Matrix mat;				// Adjacency matrix, associated with edge.
	UT_hash_handle hh;			// makes this structure hashable.
};

typedef struct Edge Edge;

/* Creates a new edge, connecting src to dest node. */
Edge* Edge_New(EdgeID id, Node *src, Node *dest, const char *relationship);

/* Adds a properties to node
 * propCount - number of new properties to add 
 * keys - array of properties keys 
 * values - array of properties values */
void Edge_Add_Properties(Edge *edge, int propCount, char **keys, SIValue *values);

/* Retrieves edge's property
 * NOTE: If the key does not exist, we return the special
 * constant value Edge_PROPERTY_NOTFOUND. */
SIValue* Edge_Get_Property(const Edge *edge, const char *key);

// Frees allocated space by given edge
void Edge_Free(Edge *edge);

#endif