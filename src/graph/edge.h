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

// Encapsulate the essence of an edge.
typedef struct {
    NodeID srcId;   // Source node ID.
    NodeID destId;  // Destination node ID.
    int relationId; // Relation type ID.
} EdgeDesc;

/* TODO: note it is possible to get into an incontinence 
 * if we set edge src node and edgeDesc srcId to different nodes. */
struct Edge {
	struct {
		EdgeID id;
		int prop_count;
		EntityProperty *properties;
	};
	EdgeDesc edgeDesc;
	char* relationship;
	Node* src;
	Node* dest;
	GrB_Matrix mat;				// Adjacency matrix, associated with edge.
	UT_hash_handle hh;			// makes this structure hashable.
};

typedef struct Edge Edge;

/* Creates a new edge, connecting src to dest node. */
Edge* Edge_New(EdgeID id, Node *src, Node *dest, const char *relationship);

// Retrieve edge source node ID.
NodeID Edge_GetSrcNodeID(const Edge *edge);

// Retrieve edge destination node ID.
NodeID Edge_GetDestNodeID(const Edge *edge);

// Retrieve edge relation ID.
int Edge_GetRelationID(const Edge *edge);

// Retrieve edge source node.
Node* Edge_GetSrcNode(Edge *e);

// Retrieve edge destination node.
Node* Edge_GetDestNode(Edge *e);

// Sets edge source node.
void Edge_SetSrcNode(Edge *e, Node *src);

// Sets edge destination node.
void Edge_SetDestNode(Edge *e, Node *dest);

// Sets edge relation type.
void Edge_SetRelationID(Edge *e, int relationId);

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