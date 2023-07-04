/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#pragma once

#include "node.h"
#include "../../value.h"
#include "graph_entity.h"

typedef int RelationID;

#define EDGE_LENGTH_INF UINT_MAX - 2


// instantiate a new edge with relation data
#define GE_NEW_LABELED_EDGE(r_str, r_id)    \
(Edge) {                                    \
	.attributes   = &NULL_ATTRIBUTE_SET,    \
	.id           = INVALID_ENTITY_ID,      \
	.relationship = (r_str),                \
	.relationID   = (r_id),                 \
	.src_id       = INVALID_ENTITY_ID,      \
	.dest_id      = INVALID_ENTITY_ID       \
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

struct Edge {
	AttributeSet *attributes;   // MUST be the first member
	EntityID id;                // Unique id, MUST be the second member
	const char *relationship;   // Label attached to edge
	RelationID relationID;      // Relation ID
	NodeID src_id;              // Source node ID
	NodeID dest_id;             // Destination node ID
};

typedef struct Edge Edge;

// retrieve edge source node ID
NodeID Edge_GetSrcNodeID
(
	const Edge *edge
);

// retrieve edge destination node ID
NodeID Edge_GetDestNodeID
(
	const Edge *edge
);

// retrieve edge relation ID
int Edge_GetRelationID
(
	const Edge *edge
);

// sets edge source node
void Edge_SetSrcNodeID
(
	Edge *e,
	NodeID id
);

// sets edge destination node
void Edge_SetDestNodeID
(
	Edge *e,
	NodeID id
);

// sets edge relation type
void Edge_SetRelationID
(
	Edge *e,
	RelationID relationID
);

// constructs a string representation of edge
void Edge_ToString
(
	const Edge *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFormat format
);

// frees allocated space by given edge
void Edge_Free
(
	Edge *edge
);
