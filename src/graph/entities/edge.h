/*
* Copyright 2018-2019 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#ifndef EDGE_H_
#define EDGE_H_

#include "node.h"
#include "../../value.h"
#include "graph_entity.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define EDGE_LENGTH_INF UINT_MAX-2

/* TODO: note it is possible to get into an inconsistency
 * if we set src and srcNodeID to different nodes. */
struct Edge {
	Entity *entity;         /* MUST be the first property of Edge. */
	char *alias;            /* Alias attached to edge. */
	char *relationship;     /* Label attached to edge. */
	int relationID;         /* Relation ID. */
	Node *src;              /* Pointer to source node. */
	Node *dest;             /* Pointer to destination node. */
	NodeID srcNodeID;       /* Source node ID. */
	NodeID destNodeID;      /* Destination node ID. */
	GrB_Matrix mat;         /* Adjacency matrix, associated with edge. */
	unsigned int minHops;   /* Minimum number of hops this edge represents. */
	unsigned int maxHops;   /* Maximum number of hops this edge represents. */
};

typedef struct Edge Edge;

/* Creates a new edge, connecting src to dest node. */
Edge *Edge_New(Node *src, Node *dest, const char *relationship,
			   const char *alias);

// Retrieve edge source node ID.
NodeID Edge_GetSrcNodeID(const Edge *edge);

// Retrieve edge destination node ID.
NodeID Edge_GetDestNodeID(const Edge *edge);

// Retrieve edge relation ID.
int Edge_GetRelationID(const Edge *edge);

// Retrieve edge source node.
Node *Edge_GetSrcNode(Edge *e);

// Retrieve edge destination node.
Node *Edge_GetDestNode(Edge *e);

// Retrieves edge matrix.
GrB_Matrix Edge_GetMatrix(Edge *e);

// Determins if this is a variable length edge.
bool Edge_VariableLength(const Edge *e);

// Reverse edge direction.
void Edge_Reverse(Edge *e);

// Sets edge source node.
void Edge_SetSrcNode(Edge *e, Node *src);

// Sets edge destination node.
void Edge_SetDestNode(Edge *e, Node *dest);

// Sets edge relation type.
void Edge_SetRelationID(Edge *e, int relationID);

// Gets a string representation of given edge.
int Edge_ToString(const Edge *e, char *buff, int buff_len);

// Frees allocated space by given edge
void Edge_Free(Edge *edge);

#endif
