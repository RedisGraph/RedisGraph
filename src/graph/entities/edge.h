/*
* Copyright 2018-2020 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "node.h"
#include "../../value.h"
#include "graph_entity.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define EDGE_LENGTH_INF UINT_MAX - 2

/* TODO: note it is possible to get into an inconsistency
 * if we set src and srcNodeID to different nodes. */
struct Edge {
	Entity *entity;             // MUST be the first member
	EntityID id;                // Unique id, MUST be the second member
	const char *relationship;   // Label attached to edge
	int relationID;             // Relation ID
	Node *src;                  // Pointer to source node
	Node *dest;                 // Pointer to destination node
	NodeID srcNodeID;           // Source node ID
	NodeID destNodeID;          // Destination node ID
	GrB_Matrix mat;             // Adjacency matrix, associated with edge
};

typedef struct Edge Edge;

/* Creates a new edge, connecting src to dest node. */
// Edge* Edge_New(Node *src, Node *dest, const char *relationship, const char *alias);

// Retrieve edge source node ID.
NodeID Edge_GetSrcNodeID(const Edge *edge); // graph.c, serializer, all_paths, replies

// Retrieve edge destination node ID.
NodeID Edge_GetDestNodeID(const Edge *edge); // graph.c, serializer, all_paths, replies

// Retrieve edge relation ID.
int Edge_GetRelationID(const Edge *edge); // graph.c, replies

// Retrieve edge source node.
Node *Edge_GetSrcNode(Edge *e); // opcreate

// Retrieve edge destination node. // opcreate
Node *Edge_GetDestNode(Edge *e);

// Retrieves edge matrix.
GrB_Matrix Edge_GetMatrix(Edge *e); // AE

// Sets edge source node.
void Edge_SetSrcNode(Edge *e, Node *src); // QG

// Sets edge destination node.
void Edge_SetDestNode(Edge *e, Node *dest); // QG

// Sets edge relation type.
void Edge_SetRelationID(Edge *e, int relationID); // QG, graph.c

/* Prints a string representation of the edge to buffer, return the string length. */
void Edge_ToString(const Edge *e, char **buffer, size_t *bufferLen, size_t *bytesWritten,
				   GraphEntityStringFromat format);

// Frees allocated space by given edge
void Edge_Free(Edge *edge);

