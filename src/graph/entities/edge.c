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
	return edge->srcNodeID;
}

NodeID Edge_GetDestNodeID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->destNodeID;
}

int Edge_GetRelationID
(
	const Edge *edge
) {
	ASSERT(edge);
	return edge->relationID;
}

Node *Edge_GetSrcNode
(
	Edge *e
) {
	ASSERT(e);
	return e->src;
}

Node *Edge_GetDestNode
(
	Edge *e
) {
	ASSERT(e);
	return e->dest;
}

void Edge_SetSrcNode
(
	Edge *e,
	Node *src
) {
	ASSERT(e && src);
	e->src = src;
	e->srcNodeID = ENTITY_GET_ID(src);
}

void Edge_SetDestNode
(
	Edge *e,
	Node *dest
) {
	ASSERT(e && dest);
	e->dest = dest;
	e->destNodeID = ENTITY_GET_ID(dest);
}

void Edge_SetRelationID
(
	Edge *e,
	int relationID
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
	GraphEntityStringFromat format
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

