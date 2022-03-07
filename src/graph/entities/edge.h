/*
* Copyright 2018-2022 Redis Labs Ltd. and Contributors
*
* This file is available under the Redis Labs Source Available License Agreement
*/

#pragma once

#include "node.h"
#include "../../value.h"
#include "graph_entity.h"
#include "../rg_matrix/rg_matrix.h"
#include "../../../deps/GraphBLAS/Include/GraphBLAS.h"

#define EDGE_LENGTH_INF UINT_MAX - 2

// instantiate a new unpopulated edge
#define GE_NEW_EDGE()                 \
(Edge) {                              \
	.entity = NULL,                   \
	.id = INVALID_ENTITY_ID,          \
	.relationship = NULL,             \
	.relationID = GRAPH_NO_RELATION,  \
	.src = NULL,                      \
	.dest = NULL,                     \
	.srcNodeID = INVALID_ENTITY_ID,   \
	.destNodeID = INVALID_ENTITY_ID,  \
	.mat = NULL                       \
}

// instantiate a new edge with relation data
#define GE_NEW_LABELED_EDGE(r_str, r_id)    \
(Edge) {                                    \
	.attributes = NULL,                     \
	.id = INVALID_ENTITY_ID,                \
	.relationship = (r_str),                \
	.relationID = (r_id),                   \
	.src = NULL,                            \
	.dest = NULL,                           \
	.srcNodeID = INVALID_ENTITY_ID,         \
	.destNodeID = INVALID_ENTITY_ID,        \
	.mat = NULL                             \
}

// resolves to relationship-type ID of the given edge
// we first attempt to retrieve it from the given entity
// then check the graph if relationship-type isn't set
#define EDGE_GET_RELATION_ID(e, g)                                                         \
__extension__({                                                                            \
	if ((e)->relationID == GRAPH_UNKNOWN_RELATION || (e)->relationID == GRAPH_NO_RELATION) \
		 (e)->relationID = Graph_GetEdgeRelation((g), (e));                                \
	(e)->relationID;                                                                       \
})

/* TODO: note it is possible to get into an inconsistency
 * if we set src and srcNodeID to different nodes. */
struct Edge {
	AttributeSet *attributes;   // MUST be the first member
	EntityID id;                // Unique id, MUST be the second member
	const char *relationship;   // Label attached to edge
	int relationID;             // Relation ID
	Node *src;                  // Pointer to source node
	Node *dest;                 // Pointer to destination node
	NodeID srcNodeID;           // Source node ID
	NodeID destNodeID;          // Destination node ID
	RG_Matrix mat;              // Adjacency matrix, associated with edge
};

typedef struct Edge Edge;

// retrieve edge source node ID
NodeID Edge_GetSrcNodeID
(
	const Edge *edge
); // graph.c, serializer, all_paths, replies

// retrieve edge destination node ID
NodeID Edge_GetDestNodeID
(
	const Edge *edge
); // graph.c, serializer, all_paths, replies

// retrieve edge relation ID
int Edge_GetRelationID
(
	const Edge *edge
); // graph.c, replies

// retrieve edge source node
Node *Edge_GetSrcNode
(
	Edge *e
); // opcreate

// retrieve edge destination node
Node *Edge_GetDestNode
(
	Edge *e
);  // opcreate

// retrieves edge matrix
RG_Matrix Edge_GetMatrix
(
	Edge *e
); // AE

// sets edge source node
void Edge_SetSrcNode
(
	Edge *e,
	Node *src
); // QG

// sets edge destination node
void Edge_SetDestNode
(
	Edge *e,
	Node *dest
); // QG

// sets edge relation type
void Edge_SetRelationID
(
	Edge *e,
	int relationID
); // QG, graph.c

// prints a string representation of the edge to buffer, return the string length
void Edge_ToString
(
	const Edge *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFromat format
);

// clones given edge
void Edge_Clone
(
	const Edge *e,
	Edge *clone
);

// frees allocated space by given edge
void Edge_Free
(
	Edge *edge
);

