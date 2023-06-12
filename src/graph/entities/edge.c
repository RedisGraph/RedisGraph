/*
 * Copyright Redis Ltd. 2018 - present
 * Licensed under your choice of the Redis Source Available License 2.0 (RSALv2) or
 * the Server Side Public License v1 (SSPLv1).
 */

#include <stdio.h>
#include <stdlib.h>

#include "edge.h"
#include "graph_entity.h"
#include "../graphcontext.h"
#include "../../query_ctx.h"

NodeID Edge_GetSrcNodeID
(
	const Edge *edge
) {
	ASSERT(edge);
	if(edge->src_id == INVALID_ENTITY_ID) {
		return INVALID_ENTITY_ID;
	}
	if(edge->src_id & MSB_MASK) {
		NodeID *id = (NodeID *)(CLEAR_MSB(edge->src_id));
		return *id;
	}
	return edge->src_id;
}

NodeID Edge_GetDestNodeID
(
	const Edge *edge
) {
	ASSERT(edge);
	if(edge->dest_id == INVALID_ENTITY_ID) {
		return INVALID_ENTITY_ID;
	}
	if(edge->dest_id & MSB_MASK) {
		NodeID *id = (NodeID *)(CLEAR_MSB(edge->dest_id));
		return *id;
	}
	return edge->dest_id;
}

int Edge_GetRelationID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->relationID;
}

void Edge_SetSrcNode
(
	Edge *e,
	Node *src
) {
	ASSERT(e && src);
	e->src_id = (NodeID)SET_MSB(&ENTITY_GET_ID(src));
}

void Edge_SetDestNode
(
	Edge *e,
	Node *dest
) {
	ASSERT(e && dest);
	e->dest_id = (NodeID)SET_MSB(&ENTITY_GET_ID(dest));
}

void Edge_SetRelationID
(
	Edge *e,
	RelationID relationID
) {
	ASSERT(e);
	e->relationID = relationID;
}

void Edge_ToString
(
	const Edge *e,
	char **buffer,
	size_t *bufferLen,
	size_t *bytesWritten,
	GraphEntityStringFormat format
) {
	GraphEntity_ToString((const GraphEntity *)e, buffer, bufferLen, bytesWritten, format, GETYPE_EDGE);
}

void Edge_Free
(
	Edge *edge
) {
	if(!edge) return;

	free(edge);
}

